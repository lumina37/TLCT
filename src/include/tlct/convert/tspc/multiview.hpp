#pragma once

#include <cmath>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "state.hpp"
#include "tlct/convert/helper.hpp"

namespace tlct::_cvt::tspc {

namespace rgs = std::ranges;

cv::Mat State::renderView(int view_row, int view_col) const
{
    constexpr int CHANNELS = 3;

    const int view_shift_x = (view_col - views_ / 2) * interval_;
    const int view_shift_y = (view_row - views_ / 2) * interval_;

    const int canvas_height = canvas_height_;
    const int canvas_width = canvas_width_;
    cv::Mat render_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_32FC3);
    cv::Mat weight_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_32FC1);

    cv::Mat rotated_patch, resized_patch;
    cv::Mat resized_patch_channels[CHANNELS];
    cv::Mat weighted_patch;

    for (const int i : rgs::views::iota(0, layout_.getMIRows())) {
        for (const int j : rgs::views::iota(0, layout_.getMICols(i))) {
            const cv::Point2d center = layout_.getMICenter(i, j);

            // Extract patch
            const double psize = TSpecificConfig::PSIZE_AMP * patchsizes_.at<int>(i, j);
            const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
            const cv::Mat& patch = getRoiImageByCenter(src_32f_, patch_center, psize);

            // Paste patch
            cv::rotate(patch, rotated_patch, cv::ROTATE_180);

            cv::resize(rotated_patch, resized_patch, {p_resize_, p_resize_}, 0, 0, cv::INTER_CUBIC);

            cv::split(resized_patch, resized_patch_channels);
            for (cv::Mat& resized_patch_channel : resized_patch_channels) {
                cv::multiply(resized_patch_channel, patch_fadeout_weight_, resized_patch_channel);
            }

            cv::merge(resized_patch_channels, CHANNELS, weighted_patch);

            // if the second bar is not out shift, then we need to shift the 1 col
            // else if the second bar is out shift, then we need to shift the 0 col
            const int right_shift = ((i % 2) ^ (int)layout_.isOutShift()) * (patch_xshift_ / 2);
            const cv::Rect roi{j * patch_xshift_ + right_shift, i * patch_yshift_, p_resize_, p_resize_};
            render_canvas(roi) += weighted_patch;
            weight_canvas(roi) += patch_fadeout_weight_;
        }
    }

    const cv::Mat cropped_rendered_image = render_canvas(canvas_crop_roi_);
    cv::Mat cropped_weight_matrix = weight_canvas(canvas_crop_roi_);
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
    cv::resize(normed_image_u8, resized_normed_image_u8, {final_width_, final_height_}, 0.0, 0.0, cv::INTER_CUBIC);

    cv::Mat view_image;
    if (layout_.getRotation() > 1e-2) {
        cv::transpose(resized_normed_image_u8, view_image);
    } else {
        view_image = std::move(resized_normed_image_u8);
    }

    return std::move(view_image);
}

} // namespace tlct::_cvt::tspc
