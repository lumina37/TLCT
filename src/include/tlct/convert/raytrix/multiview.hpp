#pragma once

#include <cmath>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/raytrix.hpp"
#include "tlct/convert/helper/grad.hpp"
#include "tlct/convert/raytrix/state.hpp"

namespace tlct::cvt::raytrix {

namespace rgs = std::ranges;

cv::Mat renderView(const State& state, int view_row, int view_col)
{
    constexpr int CHANNELS = 3;

    const auto layout = state.layout_;
    const int view_shift_x = (view_col - state.views_ / 2) * state.interval_;
    const int view_shift_y = (view_row - state.views_ / 2) * state.interval_;

    const int canvas_height = state.canvas_height_;
    const int canvas_width = state.canvas_width_;
    cv::Mat render_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_32FC3);
    cv::Mat weight_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_32FC1);

    cv::Mat rotated_patch, resized_patch;

    for (const int i : rgs::views::iota(0, layout.getMIRows())) {
        for (const int j : rgs::views::iota(0, layout.getMIMinCols())) {
            const cv::Point2d center = layout.getMICenter(i, j);
            if (center.x == 0.0 or center.y == 0.0)
                continue;

            const cv::Mat mi =
                _hp::getRoiImageByCenter(state.gray_src_, center, layout.getDiameter() / std::numbers::sqrt2);
            const double grad_weight = _hp::computeGrad(mi) + std::numeric_limits<float>::epsilon();

            // Extract patch
            const int psize = state.patchsizes_.at<int>(i, j);
            const double bound = state.bound_ / state.patch_xshift_ * psize;
            const double patch_width_with_bound = psize + bound * 2;
            const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
            const cv::Mat& patch = _hp::getRoiImageByCenter(state.src_32f_, patch_center, patch_width_with_bound);

            // Paste patch
            cv::resize(patch, resized_patch, {state.p_resize_withbound_, state.p_resize_withbound_}, 0, 0,
                       cv::INTER_CUBIC);

            cv::Mat resized_patch_channels[CHANNELS];
            cv::split(resized_patch, resized_patch_channels);
            for (cv::Mat& resized_patch_channel : resized_patch_channels) {
                cv::multiply(resized_patch_channel, state.patch_fadeout_weight_, resized_patch_channel);
            }

            cv::Mat weighted_patch;
            cv::merge(resized_patch_channels, CHANNELS, weighted_patch);

            // if the second bar is not out shift, then we need to shift the 1 col
            // else if the second bar is out shift, then we need to shift the 0 col
            const int right_shift = ((i % 2) ^ (int)layout.isOutShift()) * (state.patch_xshift_ / 2);
            const cv::Rect roi{j * state.patch_xshift_ + right_shift, i * state.patch_yshift_,
                               state.p_resize_withbound_, state.p_resize_withbound_};
            render_canvas(roi) += weighted_patch * grad_weight;
            weight_canvas(roi) += state.patch_fadeout_weight_ * grad_weight;
        }
    }

    const cv::Mat cropped_rendered_image = render_canvas(state.canvas_crop_roi_);
    cv::Mat cropped_weight_matrix = weight_canvas(state.canvas_crop_roi_);
    cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);

    cv::Mat cropped_rendered_image_channels[CHANNELS];
    cv::split(cropped_rendered_image, cropped_rendered_image_channels);
    for (cv::Mat& cropped_new_image_channel : cropped_rendered_image_channels) {
        cropped_new_image_channel /= cropped_weight_matrix;
    }
    cv::Mat normed_image;
    cv::merge(cropped_rendered_image_channels, CHANNELS, normed_image);

    cv::Mat resized_normed_image_u8, normed_image_u8;
    normed_image.convertTo(normed_image_u8, CV_8UC3);
    cv::resize(normed_image_u8, resized_normed_image_u8, {state.final_width_, state.final_height_}, 0.0, 0.0,
               cv::INTER_CUBIC);

    cv::Mat view_image;
    if (layout.getRotation() > 1e-2) {
        cv::transpose(resized_normed_image_u8, view_image);
    } else {
        view_image = std::move(resized_normed_image_u8);
    }

    return std::move(view_image);
}

} // namespace tlct::cvt::raytrix
