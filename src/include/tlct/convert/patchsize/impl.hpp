#pragma once

#include <expected>
#include <limits>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/params.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <concepts::CNeighbors TNeighbors, typename TArrange = TNeighbors::TArrange>
[[nodiscard]] static float metricOfPsize(const MIBuffers_<TArrange>& mis, const TNeighbors& neighbors,
                                         const MIBuffer& anchorMI, const float psize) {
    float minDiffRatio = std::numeric_limits<float>::max();
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis.getMI(neighbors.getNeighborIdx(direction));

        const cv::Point2f matchStep = _hp::sgn(mis.getArrange().isKepler()) * TNeighbors::getUnitShift(direction);
        const cv::Point2f cmpShift = matchStep * psize;

        const float diffRatio = compare(anchorMI, neibMI, cmpShift);
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
        }
    }

    const float metric = minDiffRatio;
    return metric;
}

template <concepts::CNeighbors TNeighbors>
[[nodiscard]] static PsizeMetric estimateWithNeighbor(const PsizeParams_<typename TNeighbors::TArrange>& params,
                                                      const MIBuffers_<typename TNeighbors::TArrange>& mis,
                                                      const TNeighbors& neighbors, const MIBuffer& anchorMI) {
    float maxIntensity = -1.f;
    typename TNeighbors::Direction maxIntensityDirection{};
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.intensity > maxIntensity) {
            maxIntensity = neibMI.intensity;
            maxIntensityDirection = direction;
        }
    }

    const MIBuffer& neibMI = mis.getMI(neighbors.getNeighborIdx(maxIntensityDirection));
    const cv::Point2f matchStep =
        _hp::sgn(mis.getArrange().isKepler()) * TNeighbors::getUnitShift(maxIntensityDirection);
    cv::Point2f cmpShift = matchStep * params.minPsize;

    float minDiffRatio = std::numeric_limits<float>::max();
    int bestPsize = params.minPsize;
    for (const int psize : rgs::views::iota(params.minPsize, params.maxPsize)) {
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

template <tcfg::concepts::CArrange TArrange>
[[nodiscard]] static float estimatePatchsize(const TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg,
                                             const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                                             const cv::Mat& prevPatchsizes, const cv::Point index) {
    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

    const MIBuffer& anchorMI = mis.getMI(index);
    const float prevPsize = prevPatchsizes.at<float>(index);

    float minMetric = std::numeric_limits<float>::max() / 2.f;
    float prevMetric = minMetric / cvtCfg.psizeShortcutFactor;
    float bestPsize;

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange, index);
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        bestPsize = prevPsize;
        prevMetric = metricOfPsize<NearNeighbors>(mis, nearNeighbors, anchorMI, prevPsize);
    } else {
        bestPsize = (float)params.minPsize;
    }

    const PsizeMetric& nearPsizeMetric = estimateWithNeighbor<NearNeighbors>(params, mis, nearNeighbors, anchorMI);
    if (nearPsizeMetric.metric < prevMetric * cvtCfg.psizeShortcutFactor) {
        minMetric = nearPsizeMetric.metric;
        bestPsize = nearPsizeMetric.psize;
    }

    if (arrange.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange, index);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbor<FarNeighbors>(params, mis, farNeighbors, anchorMI);
        if (farPsizeMetric.metric < minMetric && farPsizeMetric.metric < prevMetric * cvtCfg.psizeShortcutFactor) {
            bestPsize = farPsizeMetric.psize;
        }
    }

    return bestPsize;
}

template <tcfg::concepts::CArrange TArrange>
static std::expected<void, Error> estimatePatchsizes(const TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg,
                                                     const PsizeParams_<TArrange>& params,
                                                     const MIBuffers_<TArrange>& mis, const cv::Mat& prevPatchsizes,
                                                     cv::Mat& patchsizes) noexcept {
#pragma omp parallel for
    for (int row = 0; row < arrange.getMIRows(); row++) {
        for (int col = 0; col < arrange.getMICols(row); col++) {
            const cv::Point index{col, row};
            const float psize = estimatePatchsize<TArrange>(arrange, cvtCfg, params, mis, prevPatchsizes, index);
            patchsizes.at<float>(index) = psize;
        }
    }

    return {};
}

}  // namespace tlct::_cvt
