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
    cache.weights.create(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    _hp::MeanStddev texMeanStddev{};

    // 1-pass: stat texture intensity
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        const int rowOffset = row * arrange.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const int offset = rowOffset + col;
            const auto& mi = mis.getMI(offset);
            texMeanStddev.update(mi.intensity);
        }
    }

    // 2-pass: compute weight
    const float texIntensityMean = texMeanStddev.getMean();
    const float texIntensityStddev = texMeanStddev.getStddev();
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        const int rowOffset = row * arrange.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const int offset = rowOffset + col;
            const auto& mi = mis.getMI(offset);
            const float normedTexIntensity = (mi.intensity - texIntensityMean) / texIntensityStddev;
            cache.weights.template at<float>(row, col) = _hp::sigmoid(normedTexIntensity);
        }
    }
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool IS_MULTI_FOCUS>
static inline void renderView(const typename MvCache_<TArrange>::TChannels& srcs,
                              typename MvCache_<TArrange>::TChannels& dsts, const TArrange& arrange,
                              const MvParams_<TArrange>& params, const cv::Mat& patchsizes, MvCache_<TArrange>& cache,
                              int viewRow, int viewCol) {
    const int viewShiftX = (viewCol - params.views / 2) * params.viewInterval;
    const int viewShiftY = (viewRow - params.views / 2) * params.viewInterval;

    cv::Mat resizedPatch;
    [[maybe_unused]] cv::Mat rotatedPatch;
    cv::Mat weightedPatch;

    for (const int chanIdx : rgs::views::iota(0, (int)srcs.size())) {
        cache.renderCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache.weightCanvas.setTo(std::numeric_limits<float>::epsilon());

        for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
                // Extract patch
                const cv::Point2f center = arrange.getMICenter(row, col);
                const float psize = params.psizeInflate * patchsizes.at<float>(row, col);
                const cv::Point2f patchCenter{center.x + viewShiftX, center.y + viewShiftY};
                const cv::Mat& patch = getRoiImageByCenter(srcs[chanIdx], patchCenter, psize);

                // Paste patch
                if constexpr (IS_KEPLER) {
                    cv::rotate(patch, rotatedPatch, cv::ROTATE_180);
                    cv::resize(rotatedPatch, resizedPatch, {params.resizedPatchWidth, params.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                } else {
                    cv::resize(patch, resizedPatch, {params.resizedPatchWidth, params.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                }

                cv::multiply(resizedPatch, cache.gradBlendingWeight, weightedPatch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int rightShift = ((row % 2) ^ (int)arrange.isOutShift()) * (params.patchXShift / 2);
                const cv::Rect roi{col * params.patchXShift + rightShift, row * params.patchYShift,
                                   params.resizedPatchWidth, params.resizedPatchWidth};

                if constexpr (IS_MULTI_FOCUS) {
                    const float weight = cache.weights.template at<float>(row, col);
                    cache.renderCanvas(roi) += weightedPatch * weight;
                    cache.weightCanvas(roi) += cache.gradBlendingWeight * weight;
                } else {
                    cache.renderCanvas(roi) += weightedPatch;
                    cache.weightCanvas(roi) += cache.gradBlendingWeight;
                }
            }
        }

        cv::Mat croppedRenderedImage = cache.renderCanvas(params.canvasCropRoi);
        cv::Mat croppedWeightMatrix = cache.weightCanvas(params.canvasCropRoi);

        cv::divide(croppedRenderedImage, croppedWeightMatrix, cache.normedImage);
        cache.normedImage.convertTo(cache.u8NormedImage, CV_8UC1);
        cv::resize(cache.u8NormedImage, dsts[chanIdx], {params.outputWidth, params.outputHeight}, 0.0, 0.0,
                   cv::INTER_LINEAR_EXACT);
    }
}

}  // namespace tlct::_cvt
