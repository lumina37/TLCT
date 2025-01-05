#pragma once

#include <cmath>
#include <concepts>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"
#include "tlct/config/layout.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace _cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CLayout TLayout_, io::concepts::CFrame TFrame_, bool IS_KEPLER_, bool IS_MULTI_FOCUS_>
class State_
{
public:
    static constexpr int CHANNELS = 3;
    static constexpr bool IS_KEPLER = IS_KEPLER_;
    static constexpr bool IS_MULTI_FOCUS = IS_MULTI_FOCUS_;

    // Typename alias
    using TFrame = TFrame_;
    using TCvtConfig = tcfg::CliConfig::Convert;
    using TLayout = TLayout_;
    using TMIs = MIs_<TLayout>;
    using PsizeParams = PsizeParams_<TLayout>;
    using MvParams = MvParams_<TLayout>;
    using MvCache = MvCache_<TLayout>;

    // Constructor
    State_() = delete;
    State_(const State_& rhs) = delete;
    State_& operator=(const State_& rhs) = delete;
    TLCT_API inline State_(State_&& rhs) noexcept = default;
    TLCT_API inline State_& operator=(State_&& rhs) noexcept = default;
    TLCT_API inline State_(const State_::TLayout& layout, const State_::TCvtConfig& cvt_cfg);

    // Initialize from
    [[nodiscard]] TLCT_API static inline State_ fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Size getOutputSize() const noexcept
    {
        if (layout_.getDirection()) {
            return {mv_params_.output_height, mv_params_.output_width};
        } else {
            return {mv_params_.output_width, mv_params_.output_height};
        }
    };

    // Non-const methods
    TLCT_API inline void update(const TFrame& src);

    inline void renderInto(TFrame& dst, int view_row, int view_col) const
    {
        renderView<TLayout, IS_KEPLER, IS_MULTI_FOCUS>(mv_cache_.srcs_32f, mv_cache_.output_image_channels_u8, layout_,
                                                       mv_params_, patchsizes_, mv_cache_, view_row, view_col);

        for (int i = 0; i < MvCache::CHANNELS; i++) {
            cv::transpose(mv_cache_.output_image_channels_u8[i], mv_cache_.output_image_channels_u8[i]);
        }

        mv_cache_.output_image_channels_u8[0].copyTo(dst.getY());
        cv::resize(mv_cache_.output_image_channels_u8[1], dst.getU(), {(int)dst.getUWidth(), (int)dst.getUHeight()},
                   0.0, 0.0, cv::INTER_AREA);
        cv::resize(mv_cache_.output_image_channels_u8[2], dst.getV(), {(int)dst.getVWidth(), (int)dst.getVHeight()},
                   0.0, 0.0, cv::INTER_AREA);
    };

private:
    TLayout layout_;
    TCvtConfig cvt_cfg_;
    TMIs mis_;
    std::vector<PsizeRecord> prev_patchsizes_;
    std::vector<PsizeRecord> patchsizes_;

    PsizeParams psize_params_;
    MvParams mv_params_;
    mutable MvCache mv_cache_;
};

template <tcfg::concepts::CLayout TLayout, io::concepts::CFrame TFrame, bool IS_KEPLER, bool IS_MULTI_FOCUS>
State_<TLayout, TFrame, IS_KEPLER, IS_MULTI_FOCUS>::State_(const TLayout& layout, const TCvtConfig& cvt_cfg)
    : layout_(layout), cvt_cfg_(cvt_cfg)
{
    mis_ = TMIs::fromLayout(layout);

    prev_patchsizes_ = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols(), PsizeRecord{});
    patchsizes_ = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols());
    psize_params_ = PsizeParams_<TLayout>::fromConfigs(layout, cvt_cfg);

    mv_params_ = MvParams_<TLayout>::fromConfigs(layout, cvt_cfg);
    mv_cache_ = MvCache_<TLayout>::fromParams(mv_params_);
}

template <tcfg::concepts::CLayout TLayout, io::concepts::CFrame TFrame, bool IS_KEPLER, bool IS_MULTI_FOCUS>
State_<TLayout, TFrame, IS_KEPLER, IS_MULTI_FOCUS>
State_<TLayout, TFrame, IS_KEPLER, IS_MULTI_FOCUS>::fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg)
{
    return {layout, cvt_cfg};
}

template <tcfg::concepts::CLayout TLayout, io::concepts::CFrame TFrame, bool IS_KEPLER, bool IS_MULTI_FOCUS>
void State_<TLayout, TFrame, IS_KEPLER, IS_MULTI_FOCUS>::update(const TFrame& src)
{
    mv_cache_.raw_srcs[0] = src.getY().clone();
    mv_cache_.raw_srcs[1] = src.getU().clone();
    mv_cache_.raw_srcs[2] = src.getV().clone();

    if (layout_.getDirection()) {
        for (int i = 0; i < MvCache::CHANNELS; i++) {
            cv::transpose(mv_cache_.raw_srcs[i], mv_cache_.raw_srcs[i]);
        }
    }

    const int upsample = layout_.getUpsample();
    if (upsample != 1) [[likely]] {
        cv::resize(mv_cache_.raw_srcs[0], mv_cache_.srcs[0], {}, upsample, upsample, cv::INTER_CUBIC);
    } else {
        mv_cache_.srcs[0] = mv_cache_.raw_srcs[0];
    }

    if constexpr (TFrame::Ushift != 0) {
        const int u_upsample = upsample << TFrame::Ushift;
        cv::resize(mv_cache_.raw_srcs[1], mv_cache_.srcs[1], {}, u_upsample, u_upsample, cv::INTER_CUBIC);
    } else {
        mv_cache_.srcs[1] = mv_cache_.raw_srcs[1];
    }

    if constexpr (TFrame::Vshift != 0) {
        const int v_upsample = upsample << TFrame::Vshift;
        cv::resize(mv_cache_.raw_srcs[2], mv_cache_.srcs[2], {}, v_upsample, v_upsample, cv::INTER_CUBIC);
    } else {
        mv_cache_.srcs[2] = mv_cache_.raw_srcs[2];
    }

    for (int i = 0; i < MvCache::CHANNELS; i++) {
        mv_cache_.srcs[i].convertTo(mv_cache_.srcs_32f[i], CV_32FC1);
    }

    mis_.update(mv_cache_.srcs_32f[0]);

    std::swap(prev_patchsizes_, patchsizes_);
    estimatePatchsizes<TLayout, IS_KEPLER, IS_MULTI_FOCUS>(layout_, cvt_cfg_, psize_params_, mis_, prev_patchsizes_,
                                                           patchsizes_);
    if constexpr (IS_MULTI_FOCUS) {
        computeWeights(layout_, mis_, mv_cache_);
    }
}

} // namespace _cvt

namespace cvt {

using _cvt::State_;

} // namespace cvt

} // namespace tlct
