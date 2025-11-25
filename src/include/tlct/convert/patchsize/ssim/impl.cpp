#include <bit>
#include <format>
#include <limits>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/patchsize/ssim/functional.hpp"
#include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#include "tlct/convert/patchsize/ssim/params.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/impl.hpp"
#endif

namespace tlct::_cvt::ssim {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TPInfos&& prevPatchInfos,
                                 const TPsizeParams& params) noexcept
    : arrange_(arrange), mis_(std::move(mis)), prevPatchInfos_(std::move(prevPatchInfos)), params_(params) {}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(const TNeighbors& neighbors,
                                                        WrapSSIM& wrapAnchor) const noexcept {
    const cv::Point2f miCenter{arrange_.getRadius(), arrange_.getRadius()};
    const int maxShift = (int)(params_.patternShift * 2);

    float sumPsize = 0.0;
    float sumMetric = 0.0;
    float sumPsizeWeight = std::numeric_limits<float>::epsilon();
    float sumMetricWeight = std::numeric_limits<float>::epsilon();

    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const cv::Point2f anchorShift =
            _hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(direction) * params_.patternShift;
        const cv::Rect anchorRoi = getRoiByCenter(miCenter + anchorShift, params_.patternSize);
        wrapAnchor.updateRoi(anchorRoi);

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        WrapSSIM wrapNeib{neibMI};

        const cv::Point2f matchStep = -_hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(direction);
        cv::Point2f cmpShift = anchorShift + matchStep * params_.minPsize;

        int bestPsize = 0;
        float maxSsim = 0.0;
        for (const int psize : rgs::views::iota(params_.minPsize, maxShift)) {
            cmpShift += matchStep;

            const cv::Rect cmpRoi = getRoiByCenter(miCenter + cmpShift, params_.patternSize);
            wrapNeib.updateRoi(cmpRoi);

            const float ssim = wrapAnchor.compare(wrapNeib);
            if (ssim > maxSsim) {
                maxSsim = ssim;
                bestPsize = psize;
            }
        }

        const float weight = computeGrads(wrapAnchor.I_);
        const float metric = maxSsim * maxSsim;
        const float weightedMetric = weight * metric;
        sumPsize += bestPsize * weightedMetric;
        sumPsizeWeight += weightedMetric;
        sumMetric += weightedMetric;
        sumMetricWeight += weight;
    }

    const float clipedSumPsize = _hp::clip(sumPsize / sumPsizeWeight, (float)params_.minPsize, (float)maxShift);
    const float psize = clipedSumPsize / TNeighbors::INFLATE;
    const float metric = sumMetric / sumMetricWeight;

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(TBridge& bridge, cv::Point index) const noexcept {
    using PsizeParams = PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = prevPatchInfos_[offset].getPatchsize();

    bridge.getInfo(offset).setDhash(anchorMI.dhash);
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const uint16_t prevDhash = prevPatchInfos_[offset].getDhash();
        const uint16_t hashDist = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if (hashDist <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

    WrapSSIM wrapAnchor{anchorMI};
    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    const PsizeMetric& nearPsizeMetric = estimateWithNeighbors<NearNeighbors>(nearNeighbors, wrapAnchor);
    float maxMetric = nearPsizeMetric.metric;
    float bestPsize = nearPsizeMetric.psize;

    if (arrange_.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbors<FarNeighbors>(farNeighbors, wrapAnchor);
        if (farPsizeMetric.metric > maxMetric) {
            bestPsize = farPsizeMetric.psize;
        }
    }

    return bestPsize;
}

template <cfg::concepts::CArrange TArrange>
void PsizeImpl_<TArrange>::adjustWgtsAndPsizesForMultiFocus(TBridge& bridge) noexcept {
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = row * arrange_.getMIMaxCols() + col;
            const auto& mi = mis_.getMI(offset);

            const float weight = mi.grads + 0.01f;
            bridge.setWeight(offset, weight);
        }
    }
}

template <cfg::concepts::CArrange TArrange>
auto PsizeImpl_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<PsizeImpl_, Error> {
    auto misRes = TMIBuffers::create(arrange);
    if (!misRes) return std::unexpected{std::move(misRes.error())};
    auto& mis = misRes.value();

    std::vector<TPInfo> prevPatchInfos(arrange.getMIRows() * arrange.getMIMaxCols());

    auto paramsRes = TPsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), std::move(prevPatchInfos), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::updateBridge(const cv::Mat& src, TBridge& bridge) noexcept {
    bridge.swapInfos(prevPatchInfos_);

    auto updateRes = mis_.update(src);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int idx = 0; idx < (int)prevPatchInfos_.size(); idx++) {
        const int row = idx / arrange_.getMIMaxCols();
        const int col = idx % arrange_.getMIMaxCols();
        if (col >= arrange_.getMICols(row)) {
            continue;
        }
        const cv::Point index{col, row};
        const float psize = estimatePatchsize(bridge, index);
        bridge.getInfo(idx).setPatchsize(psize);
    }

    if (arrange_.isMultiFocus()) {
        adjustWgtsAndPsizesForMultiFocus(bridge);
    }

    return {};
}

template class PsizeImpl_<cfg::CornersArrange>;
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::ssim
