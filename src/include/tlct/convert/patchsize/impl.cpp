#include <expected>
#include <limits>
#include <numeric>
#include <ranges>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/impl.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& params) noexcept
    : arrange_(arrange), mis_(std::move(mis)), params_(params) {
    prevPatchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    patchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);

    if (arrange_.isMultiFocus()) {
        weights_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    }
}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
float PsizeImpl_<TArrange>::metricOfPsize(const TNeighbors& neighbors, const MIBuffer& anchorMI,
                                          float psize) const noexcept {
    float minDiffRatio = std::numeric_limits<float>::max();
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));

        const cv::Point2f matchStep = _hp::sgn(mis_.getArrange().isKepler()) * TNeighbors::getUnitShift(direction);
        const cv::Point2f cmpShift = matchStep * psize;

        const float diffRatio = compare(anchorMI, neibMI, cmpShift);
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
        }
    }

    const float metric = minDiffRatio;
    return metric;
}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbor(const TNeighbors& neighbors,
                                                       const MIBuffer& anchorMI) const noexcept {
    float maxIntensity = -1.f;
    typename TNeighbors::Direction maxIntensityDirection{};
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.intensity > maxIntensity) {
            maxIntensity = neibMI.intensity;
            maxIntensityDirection = direction;
        }
    }

    const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(maxIntensityDirection));
    const cv::Point2f matchStep = _hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(maxIntensityDirection);
    cv::Point2f cmpShift = matchStep * params_.minPsize;

    float minDiffRatio = std::numeric_limits<float>::max();
    int bestPsize = params_.minPsize;
    for (const int psize : rgs::views::iota(params_.minPsize, params_.maxPsize)) {
        cmpShift += matchStep;
        const float diffRatio = compare(anchorMI, neibMI, cmpShift);
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
            bestPsize = psize;
        }
    }

    const float psize = (float)bestPsize / TNeighbors::INFLATE;
    const float metric = minDiffRatio;

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(cv::Point index) const noexcept {
    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

    const MIBuffer& anchorMI = mis_.getMI(index);
    const float prevPsize = prevPatchsizes_.at<float>(index);

    float minMetric = std::numeric_limits<float>::max() / 2.f;
    float prevMetric = minMetric / params_.psizeShortcutFactor;
    float bestPsize;

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        bestPsize = prevPsize;
        prevMetric = metricOfPsize<NearNeighbors>(nearNeighbors, anchorMI, prevPsize);
    } else {
        bestPsize = (float)params_.minPsize;
    }

    const PsizeMetric& nearPsizeMetric = estimateWithNeighbor<NearNeighbors>(nearNeighbors, anchorMI);
    if (nearPsizeMetric.metric < prevMetric * params_.psizeShortcutFactor) {
        minMetric = nearPsizeMetric.metric;
        bestPsize = nearPsizeMetric.psize;
    }

    if (arrange_.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbor<FarNeighbors>(farNeighbors, anchorMI);
        if (farPsizeMetric.metric < minMetric && farPsizeMetric.metric < prevMetric * params_.psizeShortcutFactor) {
            bestPsize = farPsizeMetric.psize;
        }
    }

    return bestPsize;
}

template <cfg::concepts::CArrange TArrange>
void PsizeImpl_<TArrange>::adjustWgtsAndPsizesForMultiFocus() noexcept {
    // TODO: handle `std::bad_alloc` in this func
    _hp::MeanStddev texMeanStddev{};

    // 1-pass: stat texture intensity
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        const int rowOffset = row * arrange_.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = rowOffset + col;
            const auto& mi = mis_.getMI(offset);
            texMeanStddev.update(mi.intensity);
        }
    }

    // 2-pass: compute weight
    const float texIntensityMean = texMeanStddev.getMean();
    const float texIntensityStddev = texMeanStddev.getStddev();
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const cv::Point index{col, row};
            const auto& mi = mis_.getMI(index);
            const float currIntensity = mi.intensity;

            using TNeighbors = NearNeighbors_<TArrange>;
            const auto neighbors = TNeighbors::fromArrangeAndIndex(arrange_, index);

            std::array<float, TNeighbors::DIRECTION_NUM> neibIntensities;
            std::array<float, TNeighbors::DIRECTION_NUM> neibPsizes;
            for (const auto direction : TNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) {
                    neibIntensities[(int)direction] = -1.f;
                    neibPsizes[(int)direction] = -1.f;
                    continue;
                }
                const cv::Point neibIdx = neighbors.getNeighborIdx(direction);
                const MIBuffer& neibMI = mis_.getMI(neibIdx);
                neibIntensities[(int)direction] = neibMI.intensity;
                neibPsizes[(int)direction] = patchsizes_.at<float>(neibIdx);
            }

            const float normedTexIntensity = (mi.intensity - texIntensityMean) / texIntensityStddev;
            weights_.at<float>(row, col) = _hp::sigmoid(normedTexIntensity);

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
                weights_.at<float>(row, col) = std::numeric_limits<float>::epsilon();
                patchsizes_.at<float>(row, col) =
                    std::reduce(neibPsizes.begin(), neibPsizes.end(), 0.f) / TNeighbors::DIRECTION_NUM;
                continue;
            }

            // For blurred MI in near field.
            // These MI will have exactly 3 neighbor MIs that have clearer texture.
            // We should set their patch sizes to the average patch sizes of their clearer neighbor MIs.
            if (group0GtCount == 3 && group1GtCount == 0) {
                patchsizes_.at<float>(row, col) = (neibPsizes[0] + neibPsizes[2] + neibPsizes[4]) / 3.f;
            } else if (group0GtCount == 0 && group1GtCount == 3) {
                patchsizes_.at<float>(row, col) = (neibPsizes[1] + neibPsizes[3] + neibPsizes[5]) / 3.f;
            }
        }
    }
}

template <cfg::concepts::CArrange TArrange>
auto PsizeImpl_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<PsizeImpl_, Error> {
    auto misRes = TMIBuffers::create(arrange);
    if (!misRes) return std::unexpected{std::move(misRes.error())};
    auto& mis = misRes.value();

    auto paramsRes = PsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::update(const cv::Mat& src) noexcept {
    std::swap(prevPatchsizes_, patchsizes_);

    auto updateRes = mis_.update(src);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int row = 0; row < arrange_.getMIRows(); row++) {
        for (int col = 0; col < arrange_.getMICols(row); col++) {
            const cv::Point index{col, row};
            const float psize = estimatePatchsize(index);
            patchsizes_.at<float>(index) = psize;
        }
    }

    if (arrange_.isMultiFocus()) {
        adjustWgtsAndPsizesForMultiFocus();
    }

    return {};
}

template class PsizeImpl_<cfg::CornersArrange>;
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
