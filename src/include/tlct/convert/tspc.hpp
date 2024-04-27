#pragma once

#include <algorithm>
#include <array>
#include <filesystem>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/config/calibration/tspc.hpp"

namespace tlct::cvt {

namespace rgs = std::ranges;
namespace fs = std::filesystem;

void _Patch_Size_Cal(const cv::Mat& src, cv::Mat& patchsizes, const cfg::tspc::CalibConfig& config)
{
    const cv::Size size = config.getCentersSize();
    patchsizes = cv::Mat::zeros(size, CV_32SC1);

    cv::Mat gray_src;
    cv::cvtColor(src, gray_src, cv::COLOR_BGR2GRAY);

    for (const int i : rgs::views::iota(0, size.width)) {
        int origin_patch = 15; // TODO: Make this f**king changeable sh*t to be constant

        const auto js = rgs::views::iota(0, size.height - 1);
        for (const int j : js) {
            const cv::Point cor = config.getCenter(j, i);
            const int cor_x = cor.x;
            const int cor_y = cor.y;

            const cv::Point cor1 = config.getCenter(j + 1, i);
            const int cor_x1 = cor1.x;
            const int cor_y1 = cor1.y;
            if (cor_x1 == 0 or cor_y1 == 0)
                continue;

            const cv::Mat full_patch = gray_src(cv::Range{cor_y - 31, cor_y + 31}, cv::Range{cor_x - 31, cor_x + 31});
            const cv::Mat patch_1 = full_patch(cv::Range{19, 29}, cv::Range{19, 29});
            const auto ssim_calculator = cv::quality::QualitySSIM::create(patch_1);

            constexpr int kstart = 14;
            constexpr int kend = 33;
            auto ssim_values = std::array<double, kend - kstart>{};
            const auto ks = rgs::views::iota(kstart, kend);
            for (const int k : ks) {
                const cv::Mat full_patch_for_match = gray_src({cor_y1 - 31, cor_y1 + 31}, {cor_x1 - 31, cor_x1 + 31});
                const cv::Mat patch_2 = full_patch_for_match(cv::Range{19 + k, 29 + k}, cv::Range{19, 29});
                const double ssim_value = ssim_calculator->compute(patch_2)[0];
                ssim_values[k - kstart] = ssim_value;
            }

            // TODO: Simplify the ugly nests below
            const auto pmax = std::max_element(ssim_values.begin(), ssim_values.end());
            const double val = *pmax;
            const int ord = (int)std::distance(ssim_values.begin(), pmax);
            if (j == 0) {
                if (i == 0) {
                    origin_patch = ord + kstart;
                } else {
                    const int left_psize = patchsizes.at<int>(j, i - 1);
                    if (ssim_values[left_psize - kstart] > 0.85) {
                        origin_patch = left_psize;
                    }
                }
            } else {
                if (i == 0) {
                    if (ssim_values[origin_patch - kstart] > 0.85) {
                        origin_patch = origin_patch;
                    } else {
                        origin_patch = ord + kstart;
                    }
                } else {
                    if ((j == js.size() - 1) && (i % 2 == 0)) {
                        const int leftup_psize = patchsizes.at<int>(j - 1, i - 1);
                        const double leftup_ssim = ssim_values[leftup_psize - kstart];
                        const double last_ssim = ssim_values[origin_patch - kstart];
                        if (last_ssim > 0.85 || leftup_ssim > 0.85) {
                            if (last_ssim < leftup_ssim) {
                                origin_patch = leftup_psize;
                            } else {
                                origin_patch = origin_patch;
                            }
                        } else {
                            origin_patch = ord + kstart;
                        }
                    } else {
                        const int left_psize = patchsizes.at<int>(j, i - 1);
                        const double left_ssim = ssim_values[left_psize - kstart];
                        const double last_ssim = ssim_values[origin_patch - kstart];
                        if (last_ssim > 0.85 || left_ssim > 0.85) {
                            if (last_ssim < left_ssim) {
                                origin_patch = left_psize;
                            } else {
                                origin_patch = origin_patch;
                            }
                        } else {
                            origin_patch = ord + kstart;
                        }
                    }
                }
            }
            patchsizes.at<int>(j, i) = origin_patch;
        }
    }
    patchsizes.row(size.height - 2).copyTo(patchsizes.row(size.height - 1));
}

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

                    const cv::Point cor = config.getCenter(j, i);
                    const int cor_x = cor.x;
                    const int cor_y = cor.y;

                    if (cor_x == 0 or cor_y == 0)
                        continue;

                    // Extract patch
                    const int src_psize = zoomed_psizes.at<int>(j, i);
                    const int stitch_patch = src_psize + 2 * bound;
                    const int q_raw = (int)std::ceil((double)src_psize / 2 / zoom) + 3;
                    const cv::Mat full_patch = d_src({cor_y - q_raw + k1 - 1, cor_y + q_raw + k1 - 1},
                                                     {cor_x - q_raw + k2 - 1, cor_x + q_raw + k2 - 1});
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

} // namespace tlct::cvt