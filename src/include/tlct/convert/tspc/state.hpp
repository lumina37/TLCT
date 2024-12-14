#pragma once

#include <cmath>
#include <concepts>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"
#include "tlct/config/tspc.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/tspc/multiview.hpp"
#include "tlct/convert/tspc/patchsize.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::tspc {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <io::concepts::CFrame TFrame_>
class State_
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TFrame = TFrame_;
    using TCvtConfig = tcfg::CommonConfig::Convert;
    using TLayout = tcfg::tspc::Layout;
    using TMIs = MIs_<TLayout>;

    // Constructor
    State_() = delete;
    State_(const State_& rhs) = delete;
    State_& operator=(const State_& rhs) = delete;
    TLCT_API inline State_(State_&& rhs) noexcept = default;
    TLCT_API inline State_& operator=(State_&& rhs) noexcept = default;
    TLCT_API inline State_(const TLayout& layout, const TCvtConfig& cvt_cfg);

    // Initialize from
    [[nodiscard]] TLCT_API static inline State_ fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Size getOutputSize() const noexcept
    {
        if (layout_.isTranspose()) {
            return {mv_params_.output_height, mv_params_.output_width};
        } else {
            return {mv_params_.output_width, mv_params_.output_height};
        }
    };

    // Non-const methods
    TLCT_API inline void update(const TFrame& src);

    inline void renderInto(TFrame& dst, int view_row, int view_col) const
    {
        // mv_cache_.output_image_channels_u8[0]=dst.getY();
        renderView(mv_cache_.srcs_32f_, mv_cache_.output_image_channels_u8, layout_, patchsizes_, mv_params_, mv_cache_,
                   view_row, view_col);

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
template <io::concepts::CFrame TFrame_>
State_<TFrame_>::State_(const State_::TLayout& layout, const State_::TCvtConfig& cvt_cfg)
    : layout_(layout), cvt_cfg_(cvt_cfg)
{
    mis_ = TMIs::fromLayout(layout);

    prev_patchsizes_ = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols(), PsizeRecord{});
    patchsizes_ = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols());
    psize_params_ = PsizeParams::fromConfigs(layout, cvt_cfg);

    mv_params_ = MvParams::fromConfigs(layout, cvt_cfg);
    mv_cache_ = MvCache::fromParams(mv_params_);
}

template <io::concepts::CFrame TFrame>
State_<TFrame> State_<TFrame>::fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg)
{
    return {layout, cvt_cfg};
}

template <io::concepts::CFrame TFrame>
void State_<TFrame>::update(const TFrame& src)
{
    layout_.processInto(src.getY(), mv_cache_.rotated_srcs_[0]);
    layout_.processInto(src.getU(), mv_cache_.rotated_srcs_[1]);
    layout_.processInto(src.getV(), mv_cache_.rotated_srcs_[2]);

    mv_cache_.srcs_[0] = mv_cache_.rotated_srcs_[0];
    if constexpr (TFrame::Ushift != 0) {
        constexpr int upsample = 1 << TFrame::Ushift;
        cv::resize(mv_cache_.rotated_srcs_[1], mv_cache_.srcs_[1], {}, upsample, upsample, cv::INTER_CUBIC);
    } else {
        mv_cache_.srcs_[1] = mv_cache_.rotated_srcs_[1];
    }
    if constexpr (TFrame::Vshift != 0) {
        constexpr int upsample = 1 << TFrame::Vshift;
        cv::resize(mv_cache_.rotated_srcs_[2], mv_cache_.srcs_[2], {}, upsample, upsample, cv::INTER_CUBIC);
    } else {
        mv_cache_.srcs_[2] = mv_cache_.rotated_srcs_[2];
    }

    for (int i = 0; i < MvCache::CHANNELS; i++) {
        mv_cache_.srcs_[i].convertTo(mv_cache_.srcs_32f_[i], CV_32FC1);
    }

    mis_.update(mv_cache_.srcs_32f_[0]);

    std::swap(prev_patchsizes_, patchsizes_);
    estimatePatchsizes(layout_, cvt_cfg_, psize_params_, mis_, prev_patchsizes_, patchsizes_);
}

} // namespace tlct::_cvt::tspc
