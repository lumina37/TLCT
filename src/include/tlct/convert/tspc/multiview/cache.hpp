#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/config/tspc.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/tspc/multiview/params.hpp"

namespace tlct::_cvt::tspc {

namespace tcfg = tlct::cfg;

class MvCache
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TLayout = tcfg::tspc::Layout;
    using TChannels = std::array<cv::Mat, CHANNELS>;

    // Constructor
    inline MvCache() noexcept = default;
    inline MvCache(cv::Mat&& grad_blending_weight, cv::Mat&& render_canvas, cv::Mat&& weight_canvas)
        : grad_blending_weight(std::move(grad_blending_weight)), render_canvas(std::move(render_canvas)),
          weight_canvas(std::move(weight_canvas)), normed_image_u8(), resized_normed_image_u8() {};
    MvCache(MvCache&& rhs) noexcept = default;
    MvCache& operator=(MvCache&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static inline MvCache fromParams(const MvParams& params);

    cv::Mat grad_blending_weight;
    cv::Mat render_canvas;
    cv::Mat weight_canvas;

    TChannels rotated_srcs_;
    TChannels srcs_;
    TChannels srcs_32f_;

    cv::Mat normed_image;
    cv::Mat normed_image_u8;
    cv::Mat resized_normed_image_u8;
    TChannels output_image_channels_u8;
};

MvCache MvCache::fromParams(const MvParams& params)
{
    constexpr double GRADIENT_BLENDING_WIDTH = 0.75;
    cv::Mat grad_blending_weight = circleWithFadeoutBorder(params.resized_patch_width, GRADIENT_BLENDING_WIDTH);
    cv::Mat render_canvas{cv::Size{params.canvas_width, params.canvas_height}, CV_32FC1};
    cv::Mat weight_canvas{cv::Size{params.canvas_width, params.canvas_height}, CV_32FC1};
    return {std::move(grad_blending_weight), std::move(render_canvas), std::move(weight_canvas)};
}

} // namespace tlct::_cvt::tspc
