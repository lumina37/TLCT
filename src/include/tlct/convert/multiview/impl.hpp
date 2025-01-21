#pragma once

#include <array>
#include <cmath>
#include <limits>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview/cache.hpp"
#include "tlct/convert/multiview/params.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CLayout TLayout>
static inline void computeWeights(const TLayout& layout, const MIs_<TLayout>& mis, MvCache_<TLayout>& cache)
{
    cv::Mat texture_I(layout.getMIRows(), layout.getMIMaxCols(), CV_32FC1);
    cache.weights.create(layout.getMIRows(), layout.getMIMaxCols(), CV_32FC1);
    _hp::MeanStddev ti_meanstddev{};

    const cv::Point2d mi_center{layout.getRadius(), layout.getRadius()};
    const double mi_width = layout.getRadius();
    const cv::Rect& roi = getRoiByCenter(mi_center, mi_width);

    // 1-pass: compute texture intensity
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        const int row_offset = row * layout.getMIMaxCols();
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const int offset = row_offset + col;
            const cv::Mat& mi = mis.getMI(offset).I;
            const float curr_I = (float)textureIntensity(mi(roi));
            texture_I.at<float>(row, col) = curr_I;
            ti_meanstddev.update(curr_I);
        }
    }

    // 2-pass: compute weight
    const double ti_mean = ti_meanstddev.getMean();
    const double ti_stddev = ti_meanstddev.getStddev();
    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const float& curr_ti = texture_I.at<float>({col, row});
            const double normed_I = (curr_ti - ti_mean) / ti_stddev;
            cache.weights.template at<float>(row, col) = (float)_hp::sigmoid(normed_I);
        }
    }
}

template <tcfg::concepts::CLayout TLayout, bool IS_KEPLER, bool IS_MULTI_FOCUS>
static inline void renderView(const typename MvCache_<TLayout>::TChannels& srcs,
                              typename MvCache_<TLayout>::TChannels& dsts, const TLayout& layout,
                              const MvParams_<TLayout>& params, const std::vector<PsizeRecord>& patchsizes,
                              MvCache_<TLayout>& cache, int view_row, int view_col)
{
    const int view_shift_x = (view_col - params.views / 2) * params.view_interval;
    const int view_shift_y = (view_row - params.views / 2) * params.view_interval;

    cv::Mat resized_patch;
    [[maybe_unused]] cv::Mat rotated_patch;
    cv::Mat weighted_patch;

    for (const int chan_id : rgs::views::iota(0, (int)srcs.size())) {
        cache.render_canvas.setTo(std::numeric_limits<float>::epsilon());
        cache.weight_canvas.setTo(std::numeric_limits<float>::epsilon());

        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            const int row_offset = row * layout.getMIMaxCols();
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const int offset = row_offset + col;

                // Extract patch
                const cv::Point2d center = layout.getMICenter(row, col);
                const double psize = params.psize_inflate * patchsizes[offset].psize;
                const cv::Point2d patch_center{center.x + view_shift_x, center.y + view_shift_y};
                const cv::Mat& patch = getRoiImageByCenter(srcs[chan_id], patch_center, psize);

                // Paste patch
                if constexpr (IS_KEPLER) {
                    cv::rotate(patch, rotated_patch, cv::ROTATE_180);
                    cv::resize(rotated_patch, resized_patch, {params.resized_patch_width, params.resized_patch_width},
                               0, 0, cv::INTER_LINEAR_EXACT);
                } else {
                    cv::resize(patch, resized_patch, {params.resized_patch_width, params.resized_patch_width}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                }

                cv::multiply(resized_patch, cache.grad_blending_weight, weighted_patch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int right_shift = ((row % 2) ^ (int)layout.isOutShift()) * (params.patch_xshift / 2);
                const cv::Rect roi{col * params.patch_xshift + right_shift, row * params.patch_yshift,
                                   params.resized_patch_width, params.resized_patch_width};

                if constexpr (IS_MULTI_FOCUS) {
                    const float weight = cache.weights.template at<float>(row, col);
                    cache.render_canvas(roi) += weighted_patch * weight;
                    cache.weight_canvas(roi) += cache.grad_blending_weight * weight;
                } else {
                    cache.render_canvas(roi) += weighted_patch;
                    cache.weight_canvas(roi) += cache.grad_blending_weight;
                }
            }
        }

        cv::Mat cropped_rendered_image = cache.render_canvas(params.canvas_crop_roi);
        cv::Mat cropped_weight_matrix = cache.weight_canvas(params.canvas_crop_roi);

        cv::divide(cropped_rendered_image, cropped_weight_matrix, cache.normed_image);
        cache.normed_image.convertTo(cache.normed_image_u8, CV_8UC1);
        cv::resize(cache.normed_image_u8, dsts[chan_id], {params.output_width, params.output_height}, 0.0, 0.0,
                   cv::INTER_LINEAR_EXACT);
    }
}

} // namespace tlct::_cvt
