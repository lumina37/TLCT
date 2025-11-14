#pragma once

#include <filesystem>
#include <format>
#include <memory>

#include "tlct/config/common.hpp"
#include "tlct/convert/common/cache.hpp"
#include "tlct/convert/concepts/manager.hpp"
#include "tlct/convert/manager/traits.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io/yuv.hpp"

namespace tlct::_cvt {

namespace fs = std::filesystem;

template <concepts::CManagerTraits TTraits_>
class Manager_ {
public:
    // Typename alias
    using TTraits = TTraits_;
    using TArrange = TTraits::TArrange;
    using TCvtConfig = cfg::CliConfig::Convert;
    using TPsizeImpl = TTraits::TPsizeImpl;
    using TMvImpl = TTraits::TMvImpl;
    using TCommonCache = CommonCache_<TArrange>;
    using TBridge = TPsizeImpl::TBridge;

private:
    Manager_(std::shared_ptr<TArrange>&& pArrange, const TCvtConfig& cvtCfg,
             std::shared_ptr<TCommonCache>&& pCommonCache, TPsizeImpl&& psizeImpl, TBridge&& bridge,
             TMvImpl&& mvImpl) noexcept;

public:
    // Constructor
    Manager_() = delete;
    Manager_(const Manager_& rhs) = delete;
    Manager_& operator=(const Manager_& rhs) = delete;
    Manager_(Manager_&& rhs) noexcept = default;
    Manager_& operator=(Manager_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] static std::expected<Manager_, Error> create(const TArrange& arrange,
                                                               const TCvtConfig& cvtCfg) noexcept;

    // Const methods
    [[nodiscard]] cv::Size getOutputSize() const noexcept { return mvImpl_.getOutputSize(); }
    [[nodiscard]] std::expected<void, Error> renderInto(io::YuvPlanarFrame& dst, int viewRow,
                                                        int viewCol) const noexcept;

    // Non-const methods
    [[nodiscard]] std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;

    // Debug only
    [[nodiscard]] std::expected<void, Error> updateCommonCache(const io::YuvPlanarFrame& src) noexcept;
    [[nodiscard]] TBridge& getBridge() noexcept { return bridge_; }
    [[nodiscard]] TArrange& getArrange() noexcept { return *pArrange_; }
    [[nodiscard]] std::expected<void, Error> dumpBridge(const fs::path& dumpTo) const noexcept;
    [[nodiscard]] std::expected<void, Error> loadBridge(const fs::path& loadFrom) noexcept;

private:
    std::shared_ptr<TArrange> pArrange_;
    TCvtConfig cvtCfg_;
    std::shared_ptr<TCommonCache> pCommonCache_;
    TPsizeImpl psizeImpl_;
    TBridge bridge_;
    TMvImpl mvImpl_;
};

template <concepts::CManagerTraits TTraits>
Manager_<TTraits>::Manager_(std::shared_ptr<TArrange>&& pArrange, const TCvtConfig& cvtCfg,
                            std::shared_ptr<TCommonCache>&& pCommonCache, TPsizeImpl&& psizeImpl, TBridge&& bridge,
                            TMvImpl&& mvImpl) noexcept
    : pArrange_(std::move(pArrange)),
      cvtCfg_(cvtCfg),
      pCommonCache_(std::move(pCommonCache)),
      psizeImpl_(std::move(psizeImpl)),
      bridge_(std::move(bridge)),
      mvImpl_(std::move(mvImpl)) {}

template <concepts::CManagerTraits TTraits>
auto Manager_<TTraits>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<Manager_, Error> {
    auto pArrange = std::make_shared<TArrange>(arrange);

    auto commonCacheRes = TCommonCache::create(arrange);
    if (!commonCacheRes) return std::unexpected{std::move(commonCacheRes.error())};
    auto pCommonCache = std::make_shared<TCommonCache>(std::move(commonCacheRes.value()));

    auto psizeImplRes = TPsizeImpl::create(arrange, cvtCfg);
    if (!psizeImplRes) return std::unexpected{std::move(psizeImplRes.error())};
    auto& psizeImpl = psizeImplRes.value();

    auto bridgeRes = TBridge::create(arrange);
    if (!bridgeRes) return std::unexpected{std::move(bridgeRes.error())};
    auto& bridge = bridgeRes.value();

    auto mvImplRes = TMvImpl::create(arrange, cvtCfg, pCommonCache);
    if (!mvImplRes) return std::unexpected{std::move(mvImplRes.error())};
    auto& mvImpl = mvImplRes.value();

    return Manager_{std::move(pArrange), cvtCfg,           std::move(pCommonCache), std::move(psizeImpl),
                    std::move(bridge),   std::move(mvImpl)};
}

template <concepts::CManagerTraits TTraits>
std::expected<void, Error> Manager_<TTraits>::updateCommonCache(const io::YuvPlanarFrame& src) noexcept {
    auto commonCacheUpdateRes = pCommonCache_->update(src);
    if (!commonCacheUpdateRes) return std::unexpected{std::move(commonCacheUpdateRes.error())};
    return {};
}

template <concepts::CManagerTraits TTraits>
std::expected<void, Error> Manager_<TTraits>::update(const io::YuvPlanarFrame& src) noexcept {
    auto commonCacheUpdateRes = updateCommonCache(src);
    if (!commonCacheUpdateRes) return std::unexpected{std::move(commonCacheUpdateRes.error())};

    auto psizeUpdateRes = psizeImpl_.updateBridge(pCommonCache_->srcs[0], bridge_);
    if (!psizeUpdateRes) return std::unexpected{std::move(psizeUpdateRes.error())};

    return {};
}

template <concepts::CManagerTraits TTraits>
std::expected<void, Error> Manager_<TTraits>::renderInto(io::YuvPlanarFrame& dst, int viewRow,
                                                         int viewCol) const noexcept {
    auto renderRes = mvImpl_.renderView(bridge_, dst, viewRow, viewCol);
    if (!renderRes) return std::unexpected{std::move(renderRes.error())};
    return {};
}

template <concepts::CManagerTraits TTraits>
std::expected<void, Error> Manager_<TTraits>::dumpBridge(const fs::path& dumpTo) const noexcept {
    std::ofstream ofs{dumpTo, std::ios::binary};
    if (!ofs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}", dumpTo.string());
        return std::unexpected{Error{ECate::eSys, ofs.rdstate(), std::move(errMsg)}};
    }

    const auto& patchInfos = bridge_.getInfos();
    ofs.write((char*)patchInfos.data(), patchInfos.size() * sizeof(patchInfos[0]));
    return {};
}

template <concepts::CManagerTraits TTraits>
std::expected<void, Error> Manager_<TTraits>::loadBridge(const fs::path& loadFrom) noexcept {
    std::ifstream ifs{loadFrom, std::ios::binary};
    if (!ifs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}", loadFrom.string());
        return std::unexpected{Error{ECate::eSys, ifs.rdstate(), std::move(errMsg)}};
    }

    auto& patchInfos = bridge_.getInfos();
    ifs.read((char*)patchInfos.data(), patchInfos.size() * sizeof(patchInfos[0]));
    return {};
}

using TSPCSSIMManagerTraits =
    ManagerTraits_<cfg::CornersArrange, ssim::PsizeImpl_<cfg::CornersArrange>, pm::MvImpl_<cfg::CornersArrange>>;
using TSPCMeth0Manager = Manager_<TSPCSSIMManagerTraits>;

using TSPCCensusManagerTraits =
    ManagerTraits_<cfg::CornersArrange, census::PsizeImpl_<cfg::CornersArrange>, pm::MvImpl_<cfg::CornersArrange>>;
using TSPCMeth1Manager = Manager_<TSPCCensusManagerTraits>;

using TSPCDebugManagerTraits =
    ManagerTraits_<cfg::CornersArrange, dbg::PsizeImpl_<cfg::CornersArrange>, pm::MvImpl_<cfg::CornersArrange>>;
using TSPCDebugManager = Manager_<TSPCDebugManagerTraits>;

using RaytrixSSIMManagerTraits =
    ManagerTraits_<cfg::OffsetArrange, ssim::PsizeImpl_<cfg::OffsetArrange>, pm::MvImpl_<cfg::OffsetArrange>>;
using RaytrixMeth0Manager = Manager_<RaytrixSSIMManagerTraits>;

using RaytrixCensusManagerTraits =
    ManagerTraits_<cfg::OffsetArrange, census::PsizeImpl_<cfg::OffsetArrange>, pm::MvImpl_<cfg::OffsetArrange>>;
using RaytrixMeth1Manager = Manager_<RaytrixCensusManagerTraits>;

using RaytrixDebugManagerTraits =
    ManagerTraits_<cfg::OffsetArrange, dbg::PsizeImpl_<cfg::OffsetArrange>, pm::MvImpl_<cfg::OffsetArrange>>;
using RaytrixDebugManager = Manager_<RaytrixDebugManagerTraits>;

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/manager/impl.cpp"
#endif
