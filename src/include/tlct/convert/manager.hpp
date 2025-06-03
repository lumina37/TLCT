#pragma once

#include <ranges>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace _cvt {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange_>
class Manager_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using TPsizeImpl = PsizeImpl_<TArrange>;
    using TMvImpl = MvImpl_<TArrange>;

private:
    Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg, TPsizeImpl&& psizeImpl, TMvImpl&& MvImpl) noexcept;

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
    [[nodiscard]] std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;

    [[nodiscard]] std::expected<void, Error> renderInto(io::YuvPlanarFrame& dst, int viewRow, int viewCol) noexcept;

private:
    TArrange arrange_;
    TCvtConfig cvtCfg_;
    TPsizeImpl psizeImpl_;
    TMvImpl mvImpl_;
};

template <cfg::concepts::CArrange TArrange>
Manager_<TArrange>::Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg, TPsizeImpl&& psizeImpl,
                             TMvImpl&& mvImpl) noexcept
    : arrange_(arrange), cvtCfg_(cvtCfg), psizeImpl_(std::move(psizeImpl)), mvImpl_(std::move(mvImpl)) {}

template <cfg::concepts::CArrange TArrange>
auto Manager_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<Manager_, Error> {
    auto psizeImplRes = TPsizeImpl::create(arrange, cvtCfg);
    if (!psizeImplRes) return std::unexpected{std::move(psizeImplRes.error())};
    auto& psizeImpl = psizeImplRes.value();

    auto mvImplRes = TMvImpl::create(arrange, cvtCfg);
    if (!mvImplRes) return std::unexpected{std::move(mvImplRes.error())};
    auto& mvImpl = mvImplRes.value();

    return Manager_{arrange, cvtCfg, std::move(psizeImpl), std::move(mvImpl)};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::update(const io::YuvPlanarFrame& src) noexcept {
    {
        auto res = mvImpl_.update(src);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    {
        auto res = psizeImpl_.update(mvImpl_.getSrcChans()[0]);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::renderInto(io::YuvPlanarFrame& dst, int viewRow, int viewCol) noexcept {
    {
        auto res = mvImpl_.renderView(psizeImpl_, viewRow, viewCol);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    if (arrange_.getDirection()) {
        for (auto& dstChan : mvImpl_.getDstChans()) {
            cv::transpose(dstChan, dstChan);
        }
    }

    mvImpl_.getDstChans()[0].copyTo(dst.getY());
    cv::resize(mvImpl_.getDstChans()[1], dst.getU(), {dst.getExtent().getUWidth(), dst.getExtent().getUHeight()}, 0.0,
               0.0, cv::INTER_LINEAR);
    cv::resize(mvImpl_.getDstChans()[2], dst.getV(), {dst.getExtent().getVWidth(), dst.getExtent().getVHeight()}, 0.0,
               0.0, cv::INTER_LINEAR);

    return {};
}

}  // namespace _cvt

namespace cvt {

using _cvt::Manager_;

}  // namespace cvt

}  // namespace tlct
