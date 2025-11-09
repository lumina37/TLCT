#pragma once

#include <memory>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/convert/common/cache.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace _cvt {

template <cfg::concepts::CArrange TArrange_>
class Manager_ {
public:
    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TCommonCache = CommonCache_<TArrange>;
    using TPsizeImpl = census::PsizeImpl_<TArrange>;
    using TMvImpl = pm::MvImpl_<TArrange>;

private:
    Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg, std::shared_ptr<TCommonCache>&& pCommonCache,
             TPsizeImpl&& psizeImpl, TMvImpl&& mvImpl) noexcept;

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

    // Non-const methods
    [[nodiscard]] std::expected<void, Error> updateCommonCache(const io::YuvPlanarFrame& src) noexcept;
    [[nodiscard]] std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;
    [[nodiscard]] std::expected<void, Error> renderInto(io::YuvPlanarFrame& dst, int viewRow, int viewCol) noexcept;

    // Debug only
    [[nodiscard]] TPsizeImpl& getPsizeImpl() noexcept { return psizeImpl_; }

private:
    TArrange arrange_;
    TCvtConfig cvtCfg_;
    std::shared_ptr<TCommonCache> pCommonCache_;
    TPsizeImpl psizeImpl_;
    TMvImpl mvImpl_;
};

template <cfg::concepts::CArrange TArrange>
Manager_<TArrange>::Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg,
                             std::shared_ptr<TCommonCache>&& pCommonCache, TPsizeImpl&& psizeImpl,
                             TMvImpl&& mvImpl) noexcept
    : arrange_(arrange),
      cvtCfg_(cvtCfg),
      pCommonCache_(std::move(pCommonCache)),
      psizeImpl_(std::move(psizeImpl)),
      mvImpl_(std::move(mvImpl)) {}

template <cfg::concepts::CArrange TArrange>
auto Manager_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<Manager_, Error> {
    auto commonCacheRes = TCommonCache::create(arrange);
    if (!commonCacheRes) return std::unexpected{std::move(commonCacheRes.error())};
    auto pCommonCache = std::make_shared<TCommonCache>(std::move(commonCacheRes.value()));

    auto psizeImplRes = TPsizeImpl::create(arrange, cvtCfg);
    if (!psizeImplRes) return std::unexpected{std::move(psizeImplRes.error())};
    auto& psizeImpl = psizeImplRes.value();

    auto mvImplRes = TMvImpl::create(arrange, cvtCfg, pCommonCache);
    if (!mvImplRes) return std::unexpected{std::move(mvImplRes.error())};
    auto& mvImpl = mvImplRes.value();

    return Manager_{arrange, cvtCfg, std::move(pCommonCache), std::move(psizeImpl), std::move(mvImpl)};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::updateCommonCache(const io::YuvPlanarFrame& src) noexcept {
    auto commonCacheUpdateRes = pCommonCache_->update(src);
    if (!commonCacheUpdateRes) return std::unexpected{std::move(commonCacheUpdateRes.error())};
    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::update(const io::YuvPlanarFrame& src) noexcept {
    auto commonCacheUpdateRes = updateCommonCache(src);
    if (!commonCacheUpdateRes) return std::unexpected{std::move(commonCacheUpdateRes.error())};

    auto psizeUpdateRes = psizeImpl_.update(pCommonCache_->srcs[0]);
    if (!psizeUpdateRes) return std::unexpected{std::move(psizeUpdateRes.error())};

    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::renderInto(io::YuvPlanarFrame& dst, int viewRow, int viewCol) noexcept {
    auto renderRes = mvImpl_.renderView(psizeImpl_.getBridge(), dst, viewRow, viewCol);
    if (!renderRes) return std::unexpected{std::move(renderRes.error())};
    return {};
}

}  // namespace _cvt

namespace cvt {

using _cvt::Manager_;

}  // namespace cvt

}  // namespace tlct
