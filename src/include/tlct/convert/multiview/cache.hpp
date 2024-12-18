#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/multiview/params.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CLayout TLayout_>
class MvCache_
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TLayout = TLayout_;
    using TChannels = std::array<cv::Mat, CHANNELS>;

    // Constructor
    inline MvCache_() noexcept = default;
    inline MvCache_(cv::Mat&& grad_blending_weight, cv::Mat&& render_canvas, cv::Mat&& weight_canvas)
        : grad_blending_weight(std::move(grad_blending_weight)), render_canvas(std::move(render_canvas)),
          weight_canvas(std::move(weight_canvas)), normed_image_u8(), resized_normed_image_u8() {};
    MvCache_(MvCache_&& rhs) noexcept = default;
    MvCache_& operator=(MvCache_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static inline MvCache_ fromParams(const MvParams_<TLayout>& params);

    cv::Mat grad_blending_weight;
    cv::Mat render_canvas;
    cv::Mat weight_canvas;

    TChannels rotated_srcs_;
    TChannels srcs_;
    TChannels srcs_32f_;

    cv::Mat normed_image;
    cv::Mat normed_image_u8;
    cv::Mat resized_normed_image_u8;
    cv::Mat weights;
    TChannels output_image_channels_u8;
};

template <tcfg::concepts::CLayout TLayout>
MvCache_<TLayout> MvCache_<TLayout>::fromParams(const MvParams_<TLayout>& params)
{
    constexpr double GRADIENT_BLENDING_WIDTH = 0.75;
    cv::Mat grad_blending_weight = circleWithFadeoutBorder(params.resized_patch_width, GRADIENT_BLENDING_WIDTH);
    cv::Mat render_canvas{cv::Size{params.canvas_width, params.canvas_height}, CV_32FC1};
    cv::Mat weight_canvas{cv::Size{params.canvas_width, params.canvas_height}, CV_32FC1};
    return {std::move(grad_blending_weight), std::move(render_canvas), std::move(weight_canvas)};
}

} // namespace tlct::_cvt
