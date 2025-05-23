#include <expected>
#include <limits>
#include <ranges>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/impl.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
PatchsizeImpl_<TArrange_>::PatchsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& psizeParams)
    : arrange_(arrange), mis_(std::move(mis)), params_(psizeParams) {
    prevPatchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    patchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    weights_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
}

template <tcfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
float PatchsizeImpl_<TArrange>::metricOfPsize(const TNeighbors& neighbors, const MIBuffer& anchorMI,
                                              float psize) const {
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
template <tcfg::concepts::CArrange TArrange_>
template <concepts::CNeighbors TNeighbors>
PsizeMetric PatchsizeImpl_<TArrange_>::estimateWithNeighbor(const TNeighbors& neighbors,
                                                            const MIBuffer& anchorMI) const {
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

template <tcfg::concepts::CArrange TArrange_>
float PatchsizeImpl_<TArrange_>::estimatePatchsize(cv::Point index) const {
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

template <tcfg::concepts::CArrange TArrange>
std::expected<PatchsizeImpl_<TArrange>, typename PatchsizeImpl_<TArrange>::TError> PatchsizeImpl_<TArrange>::create(
    const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept {
    auto misRes = TMIBuffers::create(arrange);
    if (!misRes) return std::unexpected{std::move(misRes.error())};
    auto& mis = misRes.value();

    auto psizeParamsRes = PsizeParams::create(arrange, cvtCfg);
    if (!psizeParamsRes) return std::unexpected{std::move(psizeParamsRes.error())};
    auto& psizeParams = psizeParamsRes.value();

    return PatchsizeImpl_{arrange, std::move(mis), psizeParams};
}

template <tcfg::concepts::CArrange TArrange_>
std::expected<void, Error> PatchsizeImpl_<TArrange_>::step(const cv::Mat& newSrc) noexcept {
    std::swap(prevPatchsizes_, patchsizes_);

    auto updateRes = mis_.update(newSrc);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int row = 0; row < arrange_.getMIRows(); row++) {
        for (int col = 0; col < arrange_.getMICols(row); col++) {
            const cv::Point index{col, row};
            const float psize = estimatePatchsize(index);
            patchsizes_.at<float>(index) = psize;
        }
    }

    return {};
}

template class PatchsizeImpl_<tcfg::CornersArrange>;
template class PatchsizeImpl_<tcfg::OffsetArrange>;

}  // namespace tlct::_cvt
