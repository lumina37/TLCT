#include <bit>
#include <limits>
#include <numeric>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/concepts/patchsize.hpp"
#include "tlct/convert/patchsize/census/mibuffer.hpp"
#include "tlct/convert/patchsize/neighbors.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/census/impl.hpp"
#endif

namespace tlct::_cvt::census {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TPInfos&& prevPatchInfos,
                                 const TPsizeParams& params) noexcept
    : arrange_(arrange), mis_(std::move(mis)), prevPatchInfos_(std::move(prevPatchInfos)), params_(params) {}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
auto PsizeImpl_<TArrange>::maxGradDirection(const TNeighbors& neighbors) const noexcept ->
    typename TNeighbors::Direction {
    float maxGrad = -1.f;
    typename TNeighbors::Direction maxGradDirection{};
    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
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
                                                        const MIBuffer& anchorMI,
                                                        typename TNeighbors::Direction direction) const noexcept {
    const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
    const cv::Point2f matchStep = _hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(direction);

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
    using PsizeParams = PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = getPrevPatchsize(offset);

    if constexpr (DEBUG_ENABLED) {
        *bridge.getInfo(offset).getPDebugInfo() = {};
    }
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        // Early return if dhash is only slightly different
        const uint16_t prevDhash = prevPatchInfos_[offset].getDhash();
        const uint16_t dhashDiff = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if constexpr (DEBUG_ENABLED) {
            bridge.getInfo(offset).getPDebugInfo()->dhashDiff = dhashDiff;
        }
        bridge.getInfo(offset).setDhash(anchorMI.dhash);
        if (dhashDiff <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

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
    // TODO: handle `std::bad_alloc` in this func
    _hp::MeanStddev texMeanStddev{};

    // 1-pass: stat texture gradient
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        const int rowOffset = row * arrange_.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = rowOffset + col;
            const auto& mi = mis_.getMI(offset);
            texMeanStddev.update(mi.grads);
        }
    }

    // 2-pass: compute weight
    const typename TBridge::TInfos rawInfos = bridge.getInfos();
    const float texGradMean = texMeanStddev.getMean();
    const float texGradStddev = texMeanStddev.getStddev();
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = row * arrange_.getMIMaxCols() + col;
            const cv::Point index{col, row};

            const auto& mi = mis_.getMI(offset);
            const float currGrad = mi.grads;

            using TNeighbors = NearNeighbors_<TArrange>;
            using Direction = typename TNeighbors::Direction;
            const auto neighbors = TNeighbors::fromArrangeAndIndex(arrange_, index);

            std::array<float, TNeighbors::DIRECTION_NUM> neibGrads;
            std::array<float, TNeighbors::DIRECTION_NUM> neibPsizes;
            for (const auto direction : TNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) {
                    neibGrads[(int)direction] = -1.f;
                    neibPsizes[(int)direction] = -1.f;
                    continue;
                }
                const cv::Point neibIdx = neighbors.getNeighborIdx(direction);
                const int neibOffset = neibIdx.y * arrange_.getMIMaxCols() + neibIdx.x;
                const MIBuffer& neibMI = mis_.getMI(neibOffset);
                neibGrads[(int)direction] = neibMI.grads;
                neibPsizes[(int)direction] = rawInfos[neibOffset].getPatchsize();
            }

            const float normedGrad = (mi.grads - texGradMean) / (texGradStddev * 2.0f);
            const float clippedGrad = _hp::clip(normedGrad, -1.0f, 1.0f);
            const float poweredGrad = clippedGrad * clippedGrad * clippedGrad;
            bridge.setWeight(offset, poweredGrad + 1.0f);

            int group0GtCount = 0;
            int group1GtCount = 0;
            const float currGradForCmp = currGrad * 1.15f;
            group0GtCount += (int)(neibGrads[(int)Direction::UPLEFT] > currGradForCmp);
            group0GtCount += (int)(neibGrads[(int)Direction::RIGHT] > currGradForCmp);
            group0GtCount += (int)(neibGrads[(int)Direction::DOWNLEFT] > currGradForCmp);
            group1GtCount += (int)(neibGrads[(int)Direction::UPRIGHT] > currGradForCmp);
            group1GtCount += (int)(neibGrads[(int)Direction::LEFT] > currGradForCmp);
            group1GtCount += (int)(neibGrads[(int)Direction::DOWNRIGHT] > currGradForCmp);

            // For blurred MI in far field.
            // These MI will have the blurrest texture (smallest gradient) among all its neighbor MIs.
            // We should assign a smaller weight for these MI.
            if (group0GtCount + group1GtCount == 6) {
                const float newWeight = bridge.getWeight(offset) / 2.f;
                bridge.setWeight(offset, newWeight);
                const float psize = std::reduce(neibPsizes.begin(), neibPsizes.end(), 0.f) / TNeighbors::DIRECTION_NUM;
                bridge.getInfo(offset).setPatchsize(psize);

                if constexpr (DEBUG_ENABLED) {
                    bridge.getInfo(offset).getPDebugInfo()->isBlurredFar = true;
                }

                continue;
            }

            // For blurred MI in near field.
            // These MI will have exactly 3 neighbor MIs that have clearer texture.
            // We should set their patch sizes to the average patch sizes of their clearer neighbor MIs.
            if (group0GtCount == 3 && group1GtCount == 0) {
                const float psize = (neibPsizes[(int)Direction::UPLEFT] + neibPsizes[(int)Direction::RIGHT] +
                                     neibPsizes[(int)Direction::DOWNLEFT]) /
                                    3.f;
                bridge.getInfo(offset).setPatchsize(psize);
                if constexpr (DEBUG_ENABLED) {
                    bridge.getInfo(offset).getPDebugInfo()->isBlurredNear = true;
                }
            } else if (group0GtCount == 0 && group1GtCount == 3) {
                const float psize = (neibPsizes[(int)Direction::UPRIGHT] + neibPsizes[(int)Direction::LEFT] +
                                     neibPsizes[(int)Direction::DOWNLEFT]) /
                                    3.f;
                bridge.getInfo(offset).setPatchsize(psize);
                if constexpr (DEBUG_ENABLED) {
                    bridge.getInfo(offset).getPDebugInfo()->isBlurredNear = true;
                }
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

}  // namespace tlct::_cvt::census
