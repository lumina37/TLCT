#pragma once

#include <cmath>
#include <concepts>
#include <limits>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/params.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <concepts::CNeighbors TNeighbors, bool IS_KEPLER, typename TArrange = TNeighbors::TArrange>
    requires std::is_same_v<TArrange, typename TNeighbors::TArrange>
[[nodiscard]] static inline float metricOfPsize(const TArrange& arrange, const MIBuffers_<TArrange>& mis,
                                                const TNeighbors& neighbors, WrapCensus& wrapAnchor, const int psize) {
    const cv::Point2f miCenter{arrange.getRadius(), arrange.getRadius()};

    float minDiffRatio = std::numeric_limits<float>::max();
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapCensus wrapNeib{neibMI};

        const cv::Point2f matchStep = _hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction);
        const cv::Point2f cmpShift = matchStep * psize;

        const float diffRatio = wrapAnchor.compare(wrapNeib, cmpShift);
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
        }
    }

    const float metric = expf(-minDiffRatio * 2.0f);
    return metric;
}

template <concepts::CNeighbors TNeighbors, bool IS_KEPLER, typename TArrange = TNeighbors::TArrange>
    requires std::is_same_v<TArrange, typename TNeighbors::TArrange>
[[nodiscard]] static inline PsizeMetric estimateWithNeighbor(const TArrange& arrange,
                                                             const PsizeParams_<TArrange>& params,
                                                             const MIBuffers_<TArrange>& mis,
                                                             const TNeighbors& neighbors, WrapCensus& wrapAnchor) {
    const cv::Point2f miCenter{arrange.getRadius(), arrange.getRadius()};

    float sumPsize = 0.0;
    float sumMetric = 0.0;
    float sumPsizeWeight = std::numeric_limits<float>::epsilon();
    float sumMetricWeight = std::numeric_limits<float>::epsilon();
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis.getMI(neighbors.getNeighborIdx(direction));
        WrapCensus wrapNeib{neibMI};

        const cv::Point2f matchStep = _hp::sgn(IS_KEPLER) * TNeighbors::getUnitShift(direction);
        cv::Point2f cmpShift = matchStep * params.minPsize;

        int bestPsize = 0;
        float minDiffRatio = std::numeric_limits<float>::max();  // smaller is better
        for (const int psize : rgs::views::iota(params.minPsize, params.maxPsize)) {
            cmpShift += matchStep;
            const float diffRatio = wrapAnchor.compare(wrapNeib, cmpShift);
            if (diffRatio < minDiffRatio) {
                minDiffRatio = diffRatio;
                bestPsize = psize;
            }
        }

        const float weight = wrapNeib.mi.intensity;
        const float metric = expf(-minDiffRatio * 2.0f);
        const float weightedMetric = weight * metric;
        sumPsize += (float)bestPsize * weightedMetric;
        sumPsizeWeight += weightedMetric;
        sumMetric += weightedMetric;
        sumMetricWeight += weight;
    }

    const int psize = _hp::iround(sumPsize / sumPsizeWeight / TNeighbors::INFLATE);
    const float metric = sumMetric / sumMetricWeight;

    return {psize, metric};
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
[[nodiscard]] static inline int estimatePatchsize(const TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg,
                                                  const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                                                  const cv::Mat& prevPatchsizes, const cv::Point index) {
    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

    const MIBuffer& anchorMI = mis.getMI(index);
    const int prevPsize = prevPatchsizes.at<int>(index);

    float maxMetric = 0.f;
    float prevMetric = 0.f;
    int bestPsize = prevPsize;

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange, index);
    WrapCensus wrapAnchor{anchorMI};
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        prevMetric = metricOfPsize<NearNeighbors, IS_KEPLER>(arrange, mis, nearNeighbors, wrapAnchor, prevPsize);
    }

    const PsizeMetric& nearPsizeMetric =
        estimateWithNeighbor<NearNeighbors, IS_KEPLER>(arrange, params, mis, nearNeighbors, wrapAnchor);
    if (nearPsizeMetric.metric > prevMetric * cvtCfg.psizeShortcutFactor) {
        maxMetric = nearPsizeMetric.metric;
        bestPsize = nearPsizeMetric.psize;
    }

    if constexpr (USE_FAR_NEIGHBOR) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange, index);
        const PsizeMetric& farPsizeMetric =
            estimateWithNeighbor<FarNeighbors, IS_KEPLER>(arrange, params, mis, farNeighbors, wrapAnchor);
        if (farPsizeMetric.metric > maxMetric && farPsizeMetric.metric > prevMetric * cvtCfg.psizeShortcutFactor) {
            bestPsize = farPsizeMetric.psize;
        }
    }

    return bestPsize;
}

template <tcfg::concepts::CArrange TArrange, bool IS_KEPLER, bool USE_FAR_NEIGHBOR>
static inline void estimatePatchsizes(const TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg,
                                      const PsizeParams_<TArrange>& params, const MIBuffers_<TArrange>& mis,
                                      const cv::Mat& prevPatchsizes, cv::Mat& patchsizes) {
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const cv::Point index{col, row};
            const int psize = estimatePatchsize<TArrange, IS_KEPLER, USE_FAR_NEIGHBOR>(arrange, cvtCfg, params, mis,
                                                                                       prevPatchsizes, index);
            patchsizes.at<int>(index) = psize;
        }
    }
}

}  // namespace tlct::_cvt
