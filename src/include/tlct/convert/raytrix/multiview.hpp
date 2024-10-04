#pragma once

#include <cmath>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "state.hpp"
#include "tlct/convert/helper.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;

void State::renderInto(cv::Mat& dst, int view_row, int view_col) const
{
    const int view_shift_x = (view_col - views_ / 2) * view_interval_;
    const int view_shift_y = (view_row - views_ / 2) * view_interval_;

    render_canvas_.setTo(0.0);
    weight_canvas_.setTo(0.0);

    cv::Mat rotated_patch, resized_patch;
    cv::Mat resized_patch_channels[CHANNELS];
    cv::Mat weighted_patch;

    int row_offset = 0;
    for (const int i : rgs::views::iota(0, layout_.getMIRows())) {
        for (const int j : rgs::views::iota(0, layout_.getMICols(i))) {
            const int offset = row_offset + j;
            const cv::Point2d center = layout_.getMICenter(i, j);

            const auto& mi = mis_.getMI(i, j);
            const double grad_weight = grad(mi.I_) + std::numeric_limits<float>::epsilon();

            // Extract patch
            const double psize = TSpecificConfig::PSIZE_INFLATE * patchsizes_[offset].psize;
            const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
            const cv::Mat& patch = getRoiImageByCenter(src_32f_, patch_center, psize);

            // Paste patch
            cv::resize(patch, resized_patch, {resized_patch_width_, resized_patch_width_}, 0, 0, cv::INTER_CUBIC);

            cv::split(resized_patch, resized_patch_channels);
            for (cv::Mat& resized_patch_channel : resized_patch_channels) {
                cv::multiply(resized_patch_channel, grad_blending_weight_, resized_patch_channel);
            }

            cv::merge(resized_patch_channels, CHANNELS, weighted_patch);

            // if the second bar is not out shift, then we need to shift the 1 col
            // else if the second bar is out shift, then we need to shift the 0 col
            const int right_shift = ((i % 2) ^ (int)layout_.isOutShift()) * (patch_xshift_ / 2);
            const cv::Rect roi{j * patch_xshift_ + right_shift, i * patch_yshift_, resized_patch_width_,
                               resized_patch_width_};
            render_canvas_(roi) += weighted_patch * grad_weight;
            weight_canvas_(roi) += grad_blending_weight_ * grad_weight;
        }
        row_offset += layout_.getMIMaxCols();
    }

    cv::Mat cropped_rendered_image = render_canvas_(canvas_crop_roi_);
    cv::Mat cropped_weight_matrix = weight_canvas_(canvas_crop_roi_);
    cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);

    cv::split(cropped_rendered_image, cropped_rendered_image_channels_);
    for (cv::Mat& cropped_new_image_channel : cropped_rendered_image_channels_) {
        cropped_new_image_channel /= cropped_weight_matrix;
    }
    cv::Mat& normed_image = cropped_rendered_image;
    cv::merge(cropped_rendered_image_channels_, CHANNELS, normed_image);

    normed_image.convertTo(normed_image_u8_, CV_8UC3);
    cv::resize(normed_image_u8_, resized_normed_image_u8_, {output_width_, output_height_}, 0.0, 0.0, cv::INTER_CUBIC);

    if (layout_.getRotation() > std::numbers::pi / 4.0) {
        cv::transpose(resized_normed_image_u8_, dst);
    } else {
        dst = std::move(resized_normed_image_u8_);
    }
}

} // namespace tlct::_cvt::raytrix
