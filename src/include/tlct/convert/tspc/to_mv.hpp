#pragma once

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

    const double max_color = 1.0;
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

    const int zoom = layout.getUpsample();
    const int zoomto_width = 20 * zoom; // the extracted patch will be zoomed to this height
    const int zoomto_height = tlct::_hp::iround((double)zoomto_width * std::numbers::sqrt3 / 2.0);
    const int bound = 4 * zoom;
    const int zoomto_withbound = zoomto_width + 2 * bound;

    cv::Mat d_src; // convert src from 8UCn to 64FCn
    src.convertTo(d_src, CV_64FC3);

    const cv::Mat pweight_template = _hp::rectWithFadeoutBorder({zoomto_withbound, zoomto_withbound}, bound);

    const int move_range = 6 * layout.getUpsample();
    const int interval = views > 1 ? move_range * 2 / (views - 1) : 0;
    auto colviews =
        rgs::views::iota(-views / 2, views / 2 + 1) | rgs::views::transform([interval](int x) { return x * interval; });
    auto rowviews = colviews;

    int img_cnt = 1;
    for (const int colview : colviews) {
        for (const int rowview : rowviews) {
            const int canvas_width = (layout.getMIMinCols() - 1) * zoomto_width + 2 * bound + zoomto_width / 2;
            const int canvas_height = layout.getMIRows() * zoomto_height + zoomto_withbound - zoomto_height;
            const int final_width = tlct::_hp::align_to_2((int)std::round((double)canvas_width / zoom));
            const int final_height = tlct::_hp::align_to_2((int)std::round((double)canvas_height / zoom));
            cv::Mat render_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC3);
            cv::Mat weight_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC1);

            for (const int i : rgs::views::iota(0, layout.getMIRows())) {
                for (const int j : rgs::views::iota(0, layout.getMICols(i) - 1)) {
                    const cv::Point2d center = layout.getMICenter(i, j);
                    if (center.x == 0.0 or center.y == 0.0)
                        continue;

                    // Extract patch
                    const int psize = patchsizes.at<int>(i, j);
                    const int half_psize_with_bound = (int)std::ceil((double)psize / 2.0) + bound;

                    const cv::Range patch_row_range{tlct::_hp::iround(center.y) - half_psize_with_bound + rowview,
                                                    tlct::_hp::iround(center.y) + half_psize_with_bound + rowview};
                    const cv::Range patch_col_range{tlct::_hp::iround(center.x) - half_psize_with_bound + colview,
                                                    tlct::_hp::iround(center.x) + half_psize_with_bound + colview};
                    const cv::Mat patch = d_src(patch_row_range, patch_col_range);

                    cv::Mat resized_patch;
                    cv::resize(patch, resized_patch, {zoomto_withbound, zoomto_withbound}, 0, 0, cv::INTER_CUBIC);
                    cv::Mat rotated_patch;
                    cv::rotate(resized_patch, rotated_patch, cv::ROTATE_180);

                    // Paste patch
                    // if the second bar is not out shift, then we need to shift the 1 col
                    // else if the second bar is out shift, then we need to shift the 0 col
                    const int right_shift = ((i % 2) ^ (int)layout.isOutShift()) * (zoomto_width / 2);
                    cv::Rect roi{j * zoomto_width + right_shift, i * zoomto_height, zoomto_withbound, zoomto_withbound};

                    cv::Mat rotated_patch_channels[3];
                    cv::split(rotated_patch, rotated_patch_channels);
                    for (cv::Mat& rotated_patch_channel : rotated_patch_channels) {
                        cv::multiply(rotated_patch_channel, pweight_template, rotated_patch_channel);
                    }
                    cv::Mat weighted_patch;
                    cv::merge(rotated_patch_channels, 3, weighted_patch);

                    render_canvas(roi) += weighted_patch;
                    weight_canvas(roi) += pweight_template;
                }
            }

            const cv::Range crop_roi[]{cv::Range::all(),
                                       cv::Range{zoomto_withbound / 2, render_canvas.cols - zoomto_withbound / 2}};
            cv::Mat cropped_new_image = render_canvas(crop_roi);

            cv::Mat cropped_weight_matrix = weight_canvas(crop_roi);
            cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);

            cv::Mat cropped_new_image_channels[3];
            cv::split(cropped_new_image, cropped_new_image_channels);
            for (cv::Mat& cropped_new_image_channel : cropped_new_image_channels) {
                cropped_new_image_channel /= cropped_weight_matrix;
            }
            cv::Mat final_image;
            cv::merge(cropped_new_image_channels, 3, final_image);

            std::stringstream filename_s;
            filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
            img_cnt++;
            const fs::path saveto_path = saveto / filename_s.str();
            cv::Mat resized_final_image, final_image_u8;
            final_image.convertTo(final_image_u8, CV_8UC3);
            cv::resize(final_image_u8, resized_final_image, {final_width, final_height}, 0.0, 0.0, cv::INTER_CUBIC);
            cv::Mat true_final_image;
            if (layout.getRotation() != 0.0) {
                cv::transpose(resized_final_image, true_final_image);
            } else {
                true_final_image = std::move(resized_final_image);
            }
            cv::imwrite(saveto_path.string(), true_final_image);
        }
    }
}

} // namespace tlct::cvt::inline tspc
