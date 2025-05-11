#pragma once

#include <expected>
#include <limits>
#include <numeric>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview/cache.hpp"
#include "tlct/convert/multiview/params.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange>
static void adjustWgtsAndPsizesForMultiFocus(const TArrange& arrange, const MIBuffers_<TArrange>& mis,
                                             cv::Mat& patchsizes, MvCache_<TArrange>& cache) noexcept {
    // TODO: handle `std::bad_alloc` in this func
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
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const cv::Point index{col, row};
            const auto& mi = mis.getMI(index);
            const float currIntensity = mi.intensity;

            using TNeighbors = NearNeighbors_<TArrange>;
            const auto neighbors = TNeighbors::fromArrangeAndIndex(arrange, index);

            std::array<float, TNeighbors::DIRECTION_NUM> neibIntensities;
            std::array<float, TNeighbors::DIRECTION_NUM> neibPsizes;
            for (const auto direction : TNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) {
                    neibIntensities[(int)direction] = -1.f;
                    neibPsizes[(int)direction] = -1.f;
                    continue;
                }
                const cv::Point neibIdx = neighbors.getNeighborIdx(direction);
                const MIBuffer& neibMI = mis.getMI(neibIdx);
                neibIntensities[(int)direction] = neibMI.intensity;
                neibPsizes[(int)direction] = patchsizes.at<float>(neibIdx);
            }

            const float normedTexIntensity = (mi.intensity - texIntensityMean) / texIntensityStddev;
            cache.weights.template at<float>(row, col) = _hp::sigmoid(normedTexIntensity);

            int group0GtCount = 0;
            int group1GtCount = 0;
            group0GtCount += (int)(neibIntensities[0] > currIntensity);
            group1GtCount += (int)(neibIntensities[1] > currIntensity);
            group0GtCount += (int)(neibIntensities[2] > currIntensity);
            group1GtCount += (int)(neibIntensities[3] > currIntensity);
            group0GtCount += (int)(neibIntensities[4] > currIntensity);
            group1GtCount += (int)(neibIntensities[5] > currIntensity);

            // For blurred MI in far field.
            // These MI will have the blurest texture (lowest intensity) among all its neighbor MIs.
            // We should assign a small weight for these MI.
            if (group0GtCount + group1GtCount == 6) {
                cache.weights.template at<float>(row, col) = std::numeric_limits<float>::epsilon();
                patchsizes.at<float>(row, col) =
                    std::reduce(neibPsizes.begin(), neibPsizes.end(), 0.f) / TNeighbors::DIRECTION_NUM;
                continue;
            }

            // For blurred MI in near field.
            // These MI will have exactly 3 neighbor MIs that have clearer texture.
            // We should set their patch sizes to the average patch sizes of their clearer neighbor MIs.
            if (group0GtCount == 3 && group1GtCount == 0) {
                patchsizes.at<float>(row, col) = (neibPsizes[0] + neibPsizes[2] + neibPsizes[4]) / 3.f;
            } else if (group0GtCount == 0 && group1GtCount == 3) {
                patchsizes.at<float>(row, col) = (neibPsizes[1] + neibPsizes[3] + neibPsizes[5]) / 3.f;
            }
        }
    }
}

template <tcfg::concepts::CArrange TArrange>
static std::expected<void, Error> renderView(const typename MvCache_<TArrange>::TChannels& srcs,
                                             typename MvCache_<TArrange>::TChannels& dsts, const TArrange& arrange,
                                             const MvParams_<TArrange>& params, const cv::Mat& patchsizes,
                                             MvCache_<TArrange>& cache, int viewRow, int viewCol) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    const int viewShiftX = (viewCol - params.views / 2) * params.viewInterval;
    const int viewShiftY = (viewRow - params.views / 2) * params.viewInterval;

    cv::Mat resizedPatch;
    [[maybe_unused]] cv::Mat rotatedPatch;
    cv::Mat weightedPatch;

    for (const int chanIdx : rgs::views::iota(0, (int)srcs.size())) {
        cache.renderCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache.weightCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache.srcs[chanIdx].convertTo(cache.f32Chan, CV_32FC1);

        for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
                // Extract patch
                const cv::Point2f center = arrange.getMICenter(row, col);
                const float psize = params.psizeInflate * patchsizes.at<float>(row, col);
                const cv::Point2f patchCenter{center.x + viewShiftX, center.y + viewShiftY};
                const cv::Mat& patch = getRoiImageByCenter(cache.f32Chan, patchCenter, psize);

                // Paste patch
                if (arrange.isKepler()) {
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

                if (arrange.isMultiFocus()) {
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

        cv::divide(croppedRenderedImage, croppedWeightMatrix, cache.u8NormedImage, 1, CV_8UC1);
        cv::resize(cache.u8NormedImage, dsts[chanIdx], {params.outputWidth, params.outputHeight}, 0.0, 0.0,
                   cv::INTER_LINEAR_EXACT);
    }

    return {};
}

}  // namespace tlct::_cvt
