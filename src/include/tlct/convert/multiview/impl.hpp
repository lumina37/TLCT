#pragma once

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

template <tcfg::concepts::CArrange TArrange>
static inline void computeWeights(const TArrange& arrange, const MIBuffers_<TArrange>& mis, MvCache_<TArrange>& cache) {
    cv::Mat texture_I(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    cache.weights.create(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    _hp::MeanStddev ti_meanstddev{};

    const cv::Point2f mi_center{arrange.getRadius(), arrange.getRadius()};
    const float mi_width = arrange.getRadius();
    const cv::Rect& roi = getRoiByCenter(mi_center, mi_width);

    // 1-pass: compute texture intensity
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        const int row_offset = row * arrange.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const int offset = row_offset + col;
            const cv::Mat& mi = mis.getMI(offset).srcY;
            const float curr_I = textureIntensity(mi(roi));
            texture_I.at<float>(row, col) = curr_I;
            ti_meanstddev.update(curr_I);
        }
    }

    // 2-pass: compute weight
    const float ti_mean = ti_meanstddev.getMean();
    const float ti_stddev = ti_meanstddev.getStddev();
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const float& curr_ti = texture_I.at<float>({col, row});
            const float normed_I = (curr_ti - ti_mean) / ti_stddev;
            cache.weights.template at<float>(row, col) = _hp::sigmoid(normed_I);
        }
    }
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool IS_MULTI_FOCUS>
static inline void renderView(const typename MvCache_<TArrange>::TChannels& srcs,
                              typename MvCache_<TArrange>::TChannels& dsts, const TArrange& arrange,
                              const MvParams_<TArrange>& params, const cv::Mat& patchsizes, MvCache_<TArrange>& cache,
                              int view_row, int view_col) {
    const int view_shift_x = (view_col - params.views / 2) * params.viewInterval;
    const int view_shift_y = (view_row - params.views / 2) * params.viewInterval;

    cv::Mat resized_patch;
    [[maybe_unused]] cv::Mat rotated_patch;
    cv::Mat weighted_patch;

    for (const int chan_id : rgs::views::iota(0, (int)srcs.size())) {
        cache.renderCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache.weightCanvas.setTo(std::numeric_limits<float>::epsilon());

        for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
                // Extract patch
                const cv::Point2f center = arrange.getMICenter(row, col);
                const float psize = params.psizeInflate * (float)patchsizes.at<int>(row, col);
                const cv::Point2f patch_center{center.x + view_shift_x, center.y + view_shift_y};
                const cv::Mat& patch = getRoiImageByCenter(srcs[chan_id], patch_center, psize);

                // Paste patch
                if constexpr (IS_KEPLER) {
                    cv::rotate(patch, rotated_patch, cv::ROTATE_180);
                    cv::resize(rotated_patch, resized_patch, {params.resizedPatchWidth, params.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                } else {
                    cv::resize(patch, resized_patch, {params.resizedPatchWidth, params.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                }

                cv::multiply(resized_patch, cache.gradBlendingWeight, weighted_patch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int right_shift = ((row % 2) ^ (int)arrange.isOutShift()) * (params.patchXShift / 2);
                const cv::Rect roi{col * params.patchXShift + right_shift, row * params.patchYShift,
                                   params.resizedPatchWidth, params.resizedPatchWidth};

                if constexpr (IS_MULTI_FOCUS) {
                    const float weight = cache.weights.template at<float>(row, col);
                    cache.renderCanvas(roi) += weighted_patch * weight;
                    cache.weightCanvas(roi) += cache.gradBlendingWeight * weight;
                } else {
                    cache.renderCanvas(roi) += weighted_patch;
                    cache.weightCanvas(roi) += cache.gradBlendingWeight;
                }
            }
        }

        cv::Mat cropped_rendered_image = cache.renderCanvas(params.canvasCropRoi);
        cv::Mat cropped_weight_matrix = cache.weightCanvas(params.canvasCropRoi);

        cv::divide(cropped_rendered_image, cropped_weight_matrix, cache.normedImage);
        cache.normedImage.convertTo(cache.u8NormedImage, CV_8UC1);
        cv::resize(cache.u8NormedImage, dsts[chan_id], {params.outputWidth, params.outputHeight}, 0.0, 0.0,
                   cv::INTER_LINEAR_EXACT);
    }
}

}  // namespace tlct::_cvt
