#pragma once

#include <cmath>
#include <concepts>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/raytrix.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/raytrix/multiview.hpp"
#include "tlct/convert/raytrix/patchsize.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

template <io::concepts::CFrame TFrame_>
class State_
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TFrame = TFrame_;
    using TParamConfig = tcfg::ParamConfig;
    using TLayout = TParamConfig::TLayout;
    using TSpecificConfig = TLayout::TSpecificConfig;
    using TMIs = MIs_<TLayout>;

    // Constructor
    State_() = delete;
    State_(const State_& rhs) = delete;
    State_& operator=(const State_& rhs) = delete;
    TLCT_API inline State_(State_&& rhs) noexcept = default;
    TLCT_API inline State_& operator=(State_&& rhs) noexcept = default;
    TLCT_API inline State_(TLayout&& layout, TSpecificConfig&& spec_cfg, TMIs&& mis,
                           std::vector<PsizeRecord>&& prev_patchsizes, std::vector<PsizeRecord>&& patchsizes,
                           PsizeParams&& psize_params, MvParams&& mv_params, MvCache&& mv_cache)
        : layout_(std::move(layout)), spec_cfg_(std::move(spec_cfg)), mis_(std::move(mis)),
          prev_patchsizes_(std::move(prev_patchsizes)), patchsizes_(std::move(patchsizes)),
          psize_params_(std::move(psize_params)), mv_params_(std::move(mv_params)), mv_cache_(std::move(mv_cache)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline State_ fromParamCfg(const TParamConfig& param_cfg);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Size getOutputSize() const noexcept
    {
        if (layout_.getRotation() > std::numbers::pi / 4.0) {
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
    TSpecificConfig spec_cfg_;
    TMIs mis_;
    std::vector<PsizeRecord> prev_patchsizes_;
    std::vector<PsizeRecord> patchsizes_;

    PsizeParams psize_params_;
    MvParams mv_params_;
    mutable MvCache mv_cache_;
};

template <io::concepts::CFrame TFrame>
State_<TFrame> State_<TFrame>::fromParamCfg(const TParamConfig& param_cfg)
{
    const auto& calib_cfg = param_cfg.getCalibCfg();
    auto spec_cfg = param_cfg.getSpecificCfg();
    auto layout = TLayout::fromCalibAndSpecConfig(calib_cfg, spec_cfg).upsample(spec_cfg.getUpsample());

    auto mis = TMIs::fromLayout(layout);

    auto prev_patchsizes = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols(), PsizeRecord{});
    auto patchsizes = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols());
    auto psize_params = PsizeParams::fromConfigs(layout, spec_cfg);

    const int views = param_cfg.getGenericCfg().getViews();
    auto mv_params = MvParams::fromConfigs(layout, spec_cfg, views);
    auto mv_cache = MvCache::fromParams(mv_params);

    return {std::move(layout),     std::move(spec_cfg),     std::move(mis),       std::move(prev_patchsizes),
            std::move(patchsizes), std::move(psize_params), std::move(mv_params), std::move(mv_cache)};
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
    estimatePatchsizes(layout_, spec_cfg_, psize_params_, mis_, prev_patchsizes_, patchsizes_);
    computeWeights(mis_, layout_, mv_cache_);
}

} // namespace tlct::_cvt::raytrix
