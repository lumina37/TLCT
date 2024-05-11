#pragma once

#include <filesystem>
#include <numbers>
#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;
namespace fs = std::filesystem;

TLCT_API inline void _Lenslet_Rendering_zoom(const cv::Mat& src, const cfg::tspc::Layout& layout,
                                             const cv::Mat& patchsizes, const std::string_view saveto, const int views)
{
    // TODO: Polish the bullsh*t below
    fs::path saveto_dir{saveto};
    fs::create_directories(saveto_dir);

    constexpr int zoom = 4;
    constexpr int zoomto_height = 20 * zoom; // the extracted patch will be zoomed to this height
    constexpr int zoomto_width = iround((double)zoomto_height * std::numbers::sqrt3 / 2.0);
    constexpr int bound = zoom;
    constexpr int zoomto_withbound = zoomto_height + 2 * bound;

    cv::Mat d_src; // convert src from 8UCn to 64FCn
    src.convertTo(d_src, CV_64FC3);

    constexpr int move_range = 6;
    const int interval = views > 1 ? move_range * 2 / (views - 1) : 0;
    auto colviews =
        rgs::views::iota(-views / 2, views / 2 + 1) | rgs::views::transform([interval](int x) { return x * interval; });
    auto rowviews = colviews;

    const cv::Point2d center_0_0 = layout.getMICenter(0, 0);
    const cv::Point2d center_0_1 = layout.getMICenter(0, 1);
    const bool is_out_shift = center_0_1.y < center_0_0.y;

    int img_cnt = 0;
    for (const int colview : colviews) {
        for (const int rowview : rowviews) {
            const int canvas_height = (layout.getMIRows() - 1) * zoomto_height + 2 * bound + zoomto_height / 2;
            const int canvas_width = layout.getMICols() * zoomto_width + zoomto_withbound - zoomto_width;
            cv::Mat render_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC3);
            cv::Mat weight_canvas = cv::Mat::zeros(canvas_height, canvas_width, CV_64FC1);
            const cv::Mat weight_template = cv::Mat::ones({zoomto_withbound, zoomto_withbound}, CV_64FC1);

            for (const int i : rgs::views::iota(0, layout.getMICols() - 1)) {
                for (const int j : rgs::views::iota(0, layout.getMIRows() - 1)) {
                    const cv::Point2d center = layout.getMICenter(j, i);
                    if (center.x == 0.0 or center.y == 0.0)
                        continue;

                    // Extract patch
                    const int ori_psize = patchsizes.at<int>(j, i);
                    const int psize = ori_psize;

                    const int half_psize_with_bound = (int)std::ceil((double)psize / 2.0) + bound;

                    const cv::Range patch_row_range{iround(center.y) - half_psize_with_bound + colview,
                                                    iround(center.y) + half_psize_with_bound + colview + 1};
                    const cv::Range patch_col_range{iround(center.x) - half_psize_with_bound + rowview,
                                                    iround(center.x) + half_psize_with_bound + rowview + 1};
                    const cv::Mat patch = d_src(patch_row_range, patch_col_range);

                    cv::Mat resized_patch;
                    cv::resize(patch, resized_patch, {zoomto_withbound, zoomto_withbound}, 0, 0, cv::INTER_CUBIC);
                    cv::Mat rotated_patch;
                    cv::rotate(resized_patch, rotated_patch, cv::ROTATE_180);

                    // Paste patch
                    // if the second bar is not out shift, then we need to shift the 1 col
                    // else if the second bar is out shift, then we need to shift the 0 col
                    const int down_shift = (i % 2) ^ (int)is_out_shift;
                    cv::Rect roi{i * zoomto_width, j * zoomto_height + zoomto_height / 2 * down_shift, zoomto_withbound,
                                 zoomto_withbound};
                    render_canvas(roi) += rotated_patch;
                    weight_canvas(roi) += weight_template;
                }
            }

            const cv::Range crop_roi[]{cv::Range{zoomto_withbound / 2, render_canvas.rows - zoomto_withbound / 2},
                                       cv::Range::all()};
            cv::Mat cropped_new_image = render_canvas(crop_roi);
            cv::Mat cropped_weight_matrix = weight_canvas(crop_roi);
            cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);
            cv::Mat cropped_weight_matrix_3ch;
            cv::Mat cropped_weight_matrix_channels[]{cropped_weight_matrix, cropped_weight_matrix,
                                                     cropped_weight_matrix};
            cv::merge(cropped_weight_matrix_channels, 3, cropped_weight_matrix_3ch);
            cv::Mat final_image = cropped_new_image / cropped_weight_matrix_3ch;

            std::stringstream filename_s;
            filename_s << img_cnt << ".png";
            img_cnt++;
            fs::path saveto_path = saveto_dir / filename_s.str();
            cv::Mat final_image_u8;
            final_image.convertTo(final_image_u8, CV_8UC3);
            cv::imwrite(saveto_path.string(), final_image_u8);
        }
    }
}

} // namespace tlct::cvt::tspc
