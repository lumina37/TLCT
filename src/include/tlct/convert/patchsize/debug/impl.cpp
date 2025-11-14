#include <bit>
#include <limits>
#include <numeric>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/config/mitypes.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/concepts/patchsize.hpp"
#include "tlct/convert/patchsize/census/mibuffer.hpp"
#include "tlct/convert/patchsize/neighbors.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/debug/impl.hpp"
#endif

namespace tlct::_cvt::dbg {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TPInfos&& prevPatchInfos,
                                 const TPsizeParams& params) noexcept
    : arrange_(arrange), mis_(std::move(mis)), prevPatchInfos_(std::move(prevPatchInfos)), params_(params) {}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
auto PsizeImpl_<TArrange>::maxGradDirection(const TNeighbors& neighbors) const noexcept ->
    typename TNeighbors::Direction {
    const _cfg::MITypes mitypes{arrange_.isOutShift()};

    float maxGrad = -1.f;
    typename TNeighbors::Direction maxGradDirection{};
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        if constexpr (std::is_same_v<TNeighbors, NearNeighbors>) {
            const int miType = mitypes.getMIType(neighbors.getSelfIdx());
            if (miType == arrange_.getNearFocalLenType()) {
                continue;
            }
        }

        const census::MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.grads > maxGrad) {
            maxGrad = neibMI.grads;
            maxGradDirection = direction;
        }
    }

    return maxGradDirection;
}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(TBridge& bridge, const TNeighbors& neighbors,
                                                        const census::MIBuffer& anchorMI,
                                                        typename TNeighbors::Direction direction) const noexcept {
    const census::MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
    const cv::Point2f matchStep = -_hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(direction);

    std::vector<float> metrics;
    metrics.reserve(params_.maxPsize - params_.minPsize);
    float minDiffRatio = std::numeric_limits<float>::max();
    int bestPsize = params_.minPsize;
    for (const int psize : rgs::views::iota(params_.minPsize, params_.maxPsize)) {
        const cv::Point2f cmpShift = matchStep * psize;
        const float diffRatio = compare(anchorMI, neibMI, cmpShift);
        if constexpr (DEBUG_ENABLED) {
            metrics.push_back(diffRatio);
        }
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
            bestPsize = psize;
        }
    }

    const float psize = (float)bestPsize / TNeighbors::INFLATE;
    const float metric = minDiffRatio;

    if constexpr (DEBUG_ENABLED) {
        const auto index = neighbors.getSelfIdx();
        const int offset = index.y * arrange_.getMIMaxCols() + index.x;
        if constexpr (std::is_same_v<TNeighbors, FarNeighbors>) {
            bridge.getInfo(offset).getPDebugInfo()->farMetrics = std::move(metrics);
        } else {
            bridge.getInfo(offset).getPDebugInfo()->nearMetrics = std::move(metrics);
        }
    }

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(TBridge& bridge, cv::Point index) const noexcept {
    using PsizeParams = census::PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const census::MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = getPrevPatchsize(offset);

    if constexpr (DEBUG_ENABLED) {
        *bridge.getInfo(offset).getPDebugInfo() = {};
    }
    bridge.getInfo(offset).setDhash(anchorMI.dhash);
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        // Early return if dhash is only slightly different
        const uint16_t prevDhash = prevPatchInfos_[offset].getDhash();
        const uint16_t dhashDiff = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if constexpr (DEBUG_ENABLED) {
            bridge.getInfo(offset).getPDebugInfo()->dhashDiff = dhashDiff;
        }
        if (dhashDiff <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

    const _cfg::MITypes mitypes{arrange_.isOutShift()};

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    const auto nearDirection = maxGradDirection(nearNeighbors);

    const PsizeMetric& nearPsizeMetric =
        estimateWithNeighbors<NearNeighbors>(bridge, nearNeighbors, anchorMI, nearDirection);
    const float minMetric = nearPsizeMetric.metric;
    float bestPsize = nearPsizeMetric.psize;

    if (arrange_.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const auto farDirection = maxGradDirection(farNeighbors);
        const PsizeMetric& farPsizeMetric =
            estimateWithNeighbors<FarNeighbors>(bridge, farNeighbors, anchorMI, farDirection);
        if (farPsizeMetric.metric < minMetric) {
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
            const float weight = mi.grads + std::numeric_limits<float>::epsilon();

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

static_assert(concepts::CPsizeImpl<PsizeImpl_<cfg::CornersArrange>>);
template class PsizeImpl_<cfg::CornersArrange>;

static_assert(concepts::CPsizeImpl<PsizeImpl_<cfg::OffsetArrange>>);
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::dbg
