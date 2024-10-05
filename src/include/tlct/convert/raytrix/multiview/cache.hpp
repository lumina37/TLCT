#pragma once

#include <opencv2/core.hpp>

#include "tlct/config/raytrix.hpp"

#include "params.hpp"

namespace tlct::_cvt::raytrix {

namespace tcfg = tlct::cfg::raytrix;

class MvCache
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TLayout = tcfg::Layout;
    using TSpecificConfig = TLayout::TSpecificConfig;

    // Constructor
    inline MvCache(cv::Mat&& grad_blending_weight, cv::Mat&& render_canvas, cv::Mat&& weight_canvas)
        : grad_blending_weight(grad_blending_weight), render_canvas(render_canvas), weight_canvas(weight_canvas),
          cropped_rendered_image_channels(), normed_image_u8(), resized_normed_image_u8() {};
    MvCache(MvCache&& rhs) noexcept = default;
    MvCache& operator=(MvCache&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static inline MvCache fromConfigs(const TSpecificConfig& spec_cfg, const MvParams& params);

    cv::Mat grad_blending_weight;
    cv::Mat render_canvas;
    cv::Mat weight_canvas;
    cv::Mat cropped_rendered_image_channels[CHANNELS];
    cv::Mat normed_image_u8;
    cv::Mat resized_normed_image_u8;
};

MvCache MvCache::fromConfigs(const TSpecificConfig& spec_cfg, const MvParams& params)
{
    const double grad_blending_bound =
        spec_cfg.getGradientBlendingWidth() * params.patch_xshift * MvParams::PSIZE_INFLATE;
    cv::Mat grad_blending_weight =
        circleWithFadeoutBorder(params.resized_patch_width, (int)std::round(grad_blending_bound / 2));
    cv::Mat render_canvas{params.canvas_height, params.canvas_width, CV_32FC3};
    cv::Mat weight_canvas{params.canvas_height, params.canvas_width, CV_32FC1};
    return {std::move(grad_blending_weight), std::move(render_canvas), std::move(weight_canvas)};
}

} // namespace tlct::_cvt::raytrix
