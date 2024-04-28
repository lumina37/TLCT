#pragma once

#include <filesystem>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/calibration/tspc.hpp"

namespace tlct::cvt::tspc {

namespace rgs = std::ranges;
namespace fs = std::filesystem;

void _Lenslet_Rendering_zoom(const cv::Mat& src, const cfg::tspc::CalibConfig& config, const cv::Mat& patchsizes,
                             std::string_view saveto, const int views)
{
    fs::path saveto_dir{saveto};
    fs::create_directories(saveto_dir);

    constexpr int zoom = 3;
    cv::Mat zoomed_psizes = patchsizes * zoom;
    constexpr int true_p = 20 * zoom;
    constexpr int bound = zoom;
    constexpr int patch_size = true_p + 2 * bound;

    cv::Mat d_src; // convert src from 8UCn to 64FCn
    src.convertTo(d_src, CV_64FC3);
    constexpr int p1 = (int)((double)true_p / 2.0 * std::numbers::sqrt3 + 0.5); // the width of the rendered images
    constexpr int Moverange = 6;

    const int interval = views > 1 ? Moverange * 2 / (views - 1) : 0;
    auto t1 =
        rgs::views::iota(-views / 2, views / 2 + 1) | rgs::views::transform([interval](int x) { return x * interval; });
    auto t2 = t1;

    const cv::Size size = config.getCentersSize();

    int img_cnt = 0;
    for (const int k1 : t1) {
        for (const int k2 : t2) {

            constexpr int t = patch_size - p1;
            constexpr int q = patch_size / 2;
            constexpr int q1 = true_p / 2;
            cv::Mat new_image =
                cv::Mat::zeros((size.height - 1) * true_p + 2 * bound + q1, size.width * p1 + t, CV_64FC3);
            cv::Mat weight_matrix = cv::Mat::zeros(new_image.size(), CV_64FC1);
            const cv::Mat ones_weight = cv::Mat::ones({patch_size, patch_size}, CV_64FC1);

            for (const int i : rgs::views::iota(0, size.width)) {

                const auto js = rgs::views::iota(0, size.height - 1);
                for (const int j : js) {

                    const cv::Point center = config.getCenter(j, i);
                    if (center.x == 0 or center.y == 0)
                        continue;

                    // Extract patch
                    const int src_psize = zoomed_psizes.at<int>(j, i);
                    const int stitch_patch = src_psize + 2 * bound;
                    const int q_raw = (int)std::ceil((double)src_psize / 2 / zoom) + 3;
                    const cv::Mat full_patch = d_src({center.y - q_raw + k1 - 1, center.y + q_raw + k1 - 1},
                                                     {center.x - q_raw + k2 - 1, center.x + q_raw + k2 - 1});
                    cv::Mat zoomed_full_patch;
                    cv::resize(full_patch, zoomed_full_patch, {}, zoom, zoom, cv::INTER_CUBIC);
                    const cv::Mat patch =
                        zoomed_full_patch({3 * zoom, 3 * zoom + stitch_patch}, {3 * zoom, 3 * zoom + stitch_patch});
                    cv::Mat resized_patch;
                    cv::resize(patch, resized_patch, {patch_size, patch_size}, 0, 0, cv::INTER_CUBIC);
                    cv::Mat rotated_patch;
                    cv::rotate(resized_patch, rotated_patch, cv::ROTATE_180);

                    // Paste patch
                    if (i % 2 == 0) {
                        cv::Rect roi{i * p1, j * true_p, patch_size, patch_size};
                        new_image(roi) += rotated_patch;
                        weight_matrix(roi) += ones_weight;
                    } else {
                        cv::Rect roi{i * p1, j * true_p + q1, patch_size, patch_size};
                        new_image(roi) += rotated_patch;
                        weight_matrix(roi) += ones_weight;
                    }
                }
            }

            const cv::Range crop_roi[]{cv::Range{q, new_image.rows - q}, cv::Range::all()};
            cv::Mat cropped_new_image = new_image(crop_roi);
            cv::Mat cropped_weight_matrix = weight_matrix(crop_roi);
            cropped_weight_matrix.setTo(1.0, cropped_weight_matrix == 0.0);
            cv::Mat cropped_weight_matrix_3ch;
            cv::Mat cropped_weight_matrix_channels[]{cropped_weight_matrix, cropped_weight_matrix,
                                                     cropped_weight_matrix};
            cv::merge(cropped_weight_matrix_channels, 3, cropped_weight_matrix_3ch);
            cv::Mat final_image = cropped_new_image / cropped_weight_matrix_3ch;
            cv::Mat zoomed_final_image;
            cv::resize(final_image, zoomed_final_image, {}, 1.0 / zoom, 1.0 / zoom, cv::INTER_CUBIC);

            std::stringstream filename_s;
            filename_s << img_cnt << ".png";
            img_cnt++;
            fs::path saveto_path = saveto_dir / filename_s.str();
            cv::Mat final_image_u8;
            zoomed_final_image.convertTo(final_image_u8, CV_8UC3);
            cv::imwrite(saveto_path.string(), final_image_u8);
        }
    }
}

} // namespace tlct::cvt::tspc