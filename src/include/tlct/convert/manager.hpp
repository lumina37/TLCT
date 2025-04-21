#pragma once

#include <ranges>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace _cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_, bool IS_KEPLER_, bool IS_MULTI_FOCUS_>
class Manager_ {
public:
    static constexpr int CHANNELS = 3;
    static constexpr bool IS_KEPLER = IS_KEPLER_;
    static constexpr bool IS_MULTI_FOCUS = IS_MULTI_FOCUS_;

    // Typename alias
    using TCvtConfig = tcfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;
    using MvParams = MvParams_<TArrange>;
    using MvCache = MvCache_<TArrange>;

    // Constructor
    Manager_() = delete;
    Manager_(const Manager_& rhs) = delete;
    Manager_& operator=(const Manager_& rhs) = delete;
    Manager_(Manager_&& rhs) noexcept = default;
    Manager_& operator=(Manager_&& rhs) noexcept = default;
    Manager_(const Manager_::TArrange& arrange, const Manager_::TCvtConfig& cvtCfg);

    // Initialize from
    [[nodiscard]] static Manager_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg);

    // Const methods
    [[nodiscard]] cv::Size getOutputSize() const noexcept { return {mvParams_.outputWidth, mvParams_.outputHeight}; };

    // Non-const methods
    void update(const io::YuvPlanarFrame& src);

    inline void renderInto(io::YuvPlanarFrame& dst, int viewRow, int viewCol) const {
        renderView<TArrange, IS_KEPLER, IS_MULTI_FOCUS>(mvCache_.f32Srcs, mvCache_.u8OutputImageChannels, arrange_,
                                                        mvParams_, patchsizes_, mvCache_, viewRow, viewCol);

        if (arrange_.getDirection()) {
            for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
                cv::transpose(mvCache_.u8OutputImageChannels[i], mvCache_.u8OutputImageChannels[i]);
            }
        }

        mvCache_.u8OutputImageChannels[0].copyTo(dst.getY());
        cv::resize(mvCache_.u8OutputImageChannels[1], dst.getU(),
                   {dst.getExtent().getUWidth(), dst.getExtent().getUHeight()}, 0.0, 0.0, cv::INTER_LINEAR_EXACT);
        cv::resize(mvCache_.u8OutputImageChannels[2], dst.getV(),
                   {dst.getExtent().getVWidth(), dst.getExtent().getVHeight()}, 0.0, 0.0, cv::INTER_LINEAR_EXACT);
    };

private:
    TArrange arrange_;
    TCvtConfig cvtCfg_;
    TMIBuffers mis_;
    cv::Mat prevPatchsizes_;  // CV_32FC1
    cv::Mat patchsizes_;      // CV_32FC1

    PsizeParams psizeParams_;
    MvParams mvParams_;
    mutable MvCache mvCache_;
};

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool IS_MULTI_FOCUS>
Manager_<TArrange, IS_KEPLER, IS_MULTI_FOCUS>::Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg)
    : arrange_(arrange), cvtCfg_(cvtCfg) {
    mis_ = TMIBuffers::fromArrange(arrange);

    prevPatchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    patchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    psizeParams_ = PsizeParams_<TArrange>::fromConfigs(arrange, cvtCfg);

    mvParams_ = MvParams_<TArrange>::fromConfigs(arrange, cvtCfg);
    mvCache_ = MvCache_<TArrange>::fromParams(mvParams_);
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool IS_MULTI_FOCUS>
Manager_<TArrange, IS_KEPLER, IS_MULTI_FOCUS> Manager_<TArrange, IS_KEPLER, IS_MULTI_FOCUS>::fromConfigs(
    const TArrange& arrange, const TCvtConfig& cvtCfg) {
    return {arrange, cvtCfg};
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool IS_MULTI_FOCUS>
void Manager_<TArrange, IS_KEPLER, IS_MULTI_FOCUS>::update(const io::YuvPlanarFrame& src) {
    mvCache_.rawSrcs[0] = src.getY().clone();
    mvCache_.rawSrcs[1] = src.getU().clone();
    mvCache_.rawSrcs[2] = src.getV().clone();

    if (arrange_.getDirection()) {
        for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
            cv::transpose(mvCache_.rawSrcs[i], mvCache_.rawSrcs[i]);
        }
    }

    const int upsample = arrange_.getUpsample();
    if (upsample != 1) [[likely]] {
        cv::resize(mvCache_.rawSrcs[0], mvCache_.srcs[0], {}, upsample, upsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[0] = mvCache_.rawSrcs[0];
    }

    if (src.getExtent().getUShift() != 0) {
        const int uUpsample = upsample << src.getExtent().getUShift();
        cv::resize(mvCache_.rawSrcs[1], mvCache_.srcs[1], {}, uUpsample, uUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[1] = mvCache_.rawSrcs[1];
    }

    if (src.getExtent().getVShift() != 0) {
        const int vUpsample = upsample << src.getExtent().getVShift();
        cv::resize(mvCache_.rawSrcs[2], mvCache_.srcs[2], {}, vUpsample, vUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[2] = mvCache_.rawSrcs[2];
    }

    for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
        mvCache_.srcs[i].convertTo(mvCache_.f32Srcs[i], CV_32FC1);
    }

    mis_.update(mvCache_.srcs[0]);

    std::swap(prevPatchsizes_, patchsizes_);
    estimatePatchsizes<TArrange, IS_KEPLER, IS_MULTI_FOCUS>(arrange_, cvtCfg_, psizeParams_, mis_, prevPatchsizes_,
                                                            patchsizes_);
    if constexpr (IS_MULTI_FOCUS) {
        adjustWgtsAndPsizesForMFocus(arrange_, mis_, patchsizes_, mvCache_);
    }
}

}  // namespace _cvt

namespace cvt {

using _cvt::Manager_;

}  // namespace cvt

}  // namespace tlct
