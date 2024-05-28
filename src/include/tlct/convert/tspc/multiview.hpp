#pragma once

#include <cmath>
#include <filesystem>
#include <numbers>
#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"

namespace tlct::cvt::inline tspc {

namespace rgs = std::ranges;
namespace fs = std::filesystem;

namespace _hp {

static inline cv::Mat rectWithFadeoutBorder(const cv::Size size, const int border_width)
{
    cv::Mat rect = cv::Mat::ones(size, CV_64FC1);

    cv::Point lefttop{0, 0};
    cv::Point rightbot{rect.cols - 1, rect.rows - 1};
    const cv::Point vertex_step{1, 1};

    constexpr double max_color = 1.0;
    const double color_step = max_color / (border_width + 1);
    double color = color_step;
    for (int i = 0; i < border_width; i++) {
        cv::rectangle(rect, lefttop, rightbot, color, 1);
        color += color_step;
        lefttop += vertex_step;
        rightbot -= vertex_step;
    }
    return rect;
}

} // namespace _hp

TLCT_API inline void to_multiview(const cv::Mat& src, const cfg::tspc::Layout& layout, const cv::Mat& patchsizes,
                                  const fs::path& saveto, const int views)
{
    fs::create_directories(saveto);

    constexpr int channels = 3;
    const int upsample = layout.getUpsample();
    const int patch_resize_width = 20 * upsample; // the extracted patch will be zoomed to this height
    const int patch_resize_height = (int)std::round((double)patch_resize_width * std::numbers::sqrt3 / 2.0);
    const int bound = 4 * upsample;
    const int p_resize_width_withbound = patch_resize_width + 2 * bound;

    cv::Mat d_src; // convert src from 8UCn to 64FCn
    src.convertTo(d_src, CV_64FC3);

    const cv::Mat patch_fadeout_weight =
        _hp::rectWithFadeoutBorder({p_resize_width_withbound, p_resize_width_withbound}, bound);

    const int move_range = (int)std::round(12.0 / 70.0 * layout.getDiameter());
    const int interval = views > 1 ? move_range / (views - 1) : 0;

    int img_cnt = 1;
    for (const int view_col : rgs::views::iota(0, views)) {
        for (const int view_row : rgs::views::iota(0, views)) {
            const int view_shift_x = (view_col - views / 2) * interval;
            const int view_shift_y = (view_row - views / 2) * interval;

            const int canvas_width =
                (layout.getMIMinCols() - 1) * patch_resize_width + 2 * bound + patch_resize_width / 2;
            const int canvas_height =
                layout.getMIRows() * patch_resize_height + p_resize_width_withbound - patch_resize_height;
            const int final_width = tlct::_hp::align_to_2((int)std::round((double)canvas_width / upsample));
            const int final_height = tlct::_hp::align_to_2((int)std::round((double)canvas_height / upsample));
            cv::Mat render_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC3);
            cv::Mat weight_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC1);

            cv::Mat rotated_patch, resized_patch;

            for (const int i : rgs::views::iota(0, layout.getMIRows())) {
                for (const int j : rgs::views::iota(0, layout.getMIMinCols() - 1)) {
                    const cv::Point2d center = layout.getMICenter(i, j);
                    if (center.x == 0.0 or center.y == 0.0)
                        continue;

                    // Extract patch
                    const int psize = patchsizes.at<int>(i, j);
                    const int psize_with_bound = psize + bound * 2;
                    const int patch_lefttop_x = (int)std::round(center.x - (double)psize / 2.0 - bound + view_shift_x);
                    const int patch_lefttop_y = (int)std::round(center.y - (double)psize / 2.0 - bound + view_shift_y);
                    const cv::Mat patch = d_src({patch_lefttop_x, patch_lefttop_y, psize_with_bound, psize_with_bound});

                    // Paste patch
                    cv::rotate(patch, rotated_patch, cv::ROTATE_180);
                    cv::resize(rotated_patch, resized_patch, {p_resize_width_withbound, p_resize_width_withbound}, 0, 0,
                               cv::INTER_CUBIC);

                    cv::Mat resized_patch_channels[channels];
                    cv::split(resized_patch, resized_patch_channels);
                    for (cv::Mat& resized_patch_channel : resized_patch_channels) {
                        cv::multiply(resized_patch_channel, patch_fadeout_weight, resized_patch_channel);
                    }

                    cv::Mat weighted_patch;
                    cv::merge(resized_patch_channels, channels, weighted_patch);

                    // if the second bar is not out shift, then we need to shift the 1 col
                    // else if the second bar is out shift, then we need to shift the 0 col
                    const int right_shift = ((i % 2) ^ (int)layout.isOutShift()) * (patch_resize_width / 2);
                    const cv::Rect roi{j * patch_resize_width + right_shift, i * patch_resize_height,
                                       p_resize_width_withbound, p_resize_width_withbound};
                    render_canvas(roi) += weighted_patch;
                    weight_canvas(roi) += patch_fadeout_weight;
                }
            }

            const cv::Range crop_roi[]{cv::Range::all(), cv::Range{p_resize_width_withbound / 2,
                                                                   render_canvas.cols - p_resize_width_withbound / 2}};
            const cv::Mat cropped_rendered_image = render_canvas(crop_roi);

            cv::Mat cropped_weight_matrix = weight_canvas(crop_roi);
            cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);

            cv::Mat cropped_rendered_image_channels[channels];
            cv::split(cropped_rendered_image, cropped_rendered_image_channels);
            for (cv::Mat& cropped_new_image_channel : cropped_rendered_image_channels) {
                cropped_new_image_channel /= cropped_weight_matrix;
            }
            cv::Mat normed_image;
            cv::merge(cropped_rendered_image_channels, channels, normed_image);

            cv::Mat resized_normed_image_u8, normed_image_u8;
            normed_image.convertTo(normed_image_u8, CV_8UC3);
            cv::resize(normed_image_u8, resized_normed_image_u8, {final_width, final_height}, 0.0, 0.0,
                       cv::INTER_CUBIC);
            cv::Mat final_image;
            if (layout.getRotation() != 0.0) {
                cv::transpose(resized_normed_image_u8, final_image);
            } else {
                final_image = std::move(resized_normed_image_u8);
            }

            std::stringstream filename_s;
            filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
            img_cnt++;
            const fs::path saveto_path = saveto / filename_s.str();
            cv::imwrite(saveto_path.string(), final_image);
        }
    }
}

} // namespace tlct::cvt::inline tspc
