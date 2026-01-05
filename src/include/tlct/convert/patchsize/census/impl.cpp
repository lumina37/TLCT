#include <limits>
#include <queue>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/common/config.h"
#include "tlct/config.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/concepts/patchsize.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/patchsize/census/mibuffer.hpp"
#include "tlct/convert/patchsize/census/ssim.hpp"
#include "tlct/convert/patchsize/helper/neighbors.hpp"
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
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TMIBuffers&& prevMis,
                                 TPInfos&& prevPatchInfos, const TPsizeParams& params) noexcept
    : arrange_(arrange),
      mis_(std::move(mis)),
      prevMis_(std::move(prevMis)),
      prevPatchInfos_(std::move(prevPatchInfos)),
      params_(params) {}

template <cfg::concepts::CArrange TArrange>
template <concepts::CNeighbors TNeighbors>
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(const TNeighbors& neighbors,
                                                        const MIBuffer& anchorMI) const noexcept {
    float sumPsize = 0.f;
    float sumMetric = 0.f;
    int neibCount = 0;

    for (const auto direction : TNeighbors::DIRECTIONS) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        const cv::Point2f matchStep = -_hp::sgn(arrange_.isKepler()) * TNeighbors::getUnitShift(direction);

        int bestPsize = 0;
        float maxMetric = std::numeric_limits<float>::lowest();
        for (const int psize : rgs::views::iota(params_.minPsize, params_.maxPsize)) {
            const cv::Point2f cmpShift = matchStep * psize;
            const float metric = compare(anchorMI, neibMI, cmpShift);
            if (metric > maxMetric) {
                maxMetric = metric;
                bestPsize = psize;
            }
        }

        sumPsize += (float)bestPsize;
        sumMetric += maxMetric;
        neibCount++;
    }

    const float psize = sumPsize / (float)neibCount;
    const float metric = sumMetric / (float)neibCount;

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(TBridge& bridge, cv::Point index) const noexcept {
    using PsizeParams = PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = getPrevPatchsize(offset);

    if constexpr (TLCT_ENABLE_DEBUG) {
        *bridge.getInfo(offset).getPDebugInfo() = {};
    }

    WrapSSIM wrapAnchor{anchorMI};
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const cv::Point censusCenter{anchorMI.censusMap.cols / 2, anchorMI.censusMap.rows / 2};
        const cv::Rect roi =
            getRoiByCenter(censusCenter, (int)((float)anchorMI.censusMap.cols / std::numbers::sqrt2_v<float>));
        wrapAnchor.updateRoi(roi);

        const MIBuffer& prevMI = prevMis_.getMI(offset);
        WrapSSIM wrapPrev{prevMI};
        wrapPrev.updateRoi(roi);

        const float ssim = wrapAnchor.compare(wrapPrev);
        if (ssim >= params_.psizeShortcutThreshold) {
            bridge.getInfo(offset).setInherited(true);
            return prevPsize;
        }
    }

    const cfg::MITypes mitypes{arrange_.isOutShift()};
    const int miType = mitypes.getMIType(index);

    float bestPsize;
    if (arrange_.isMultiFocus() && miType == arrange_.getNearFocalLenType()) {
        // if the MI type is for near focal, then only search its far neighbors
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbors<FarNeighbors>(farNeighbors, anchorMI);
        bestPsize = farPsizeMetric.psize;
    } else {
        const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& nearPsizeMetric = estimateWithNeighbors<NearNeighbors>(nearNeighbors, anchorMI);
        bestPsize = nearPsizeMetric.psize;
    }

    return bestPsize;
}

template <cfg::concepts::CArrange TArrange>
void PsizeImpl_<TArrange>::adjustWgtsAndPsizesForMultiFocus(TBridge& bridge) noexcept {
    // stat
    const int approxMICount = arrange_.getMIRows() * arrange_.getMIMaxCols();
    const int heapSize = approxMICount / cfg::MITypes::LEN_TYPE_NUM / 32;

    using Elem = std::pair<float, float>;
    using Heap = std::priority_queue<Elem>;
    std::array<Heap, cfg::MITypes::LEN_TYPE_NUM> heaps;

    const auto insert = [&](const int miType, const float grads, const float psize) {
        auto& heap = heaps[miType];
        if (heap.size() < heapSize) {
            heap.push({grads, psize});
        } else if (grads > heap.top().first) {
            heap.pop();
            heap.push({grads, psize});
        }
    };

    const cfg::MITypes miTypes{arrange_.isOutShift()};
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        const int rowOffset = row * arrange_.getMIMaxCols();
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = rowOffset + col;

            const auto& mi = mis_.getMI(offset);

            const float weight = mi.grads + 0.01f;
            bridge.setWeight(offset, weight);

            const int miType = miTypes.getMIType(row, col);
            const float psize = bridge.getInfo(row, col).getPatchsize();
            insert(miType, mi.grads, psize);
        }
    }

    struct PsizeInfo {
        float mean;
        float stddev;

        [[nodiscard]] float minPsize() const { return mean - 2.f * stddev; }
        [[nodiscard]] float maxPsize() const { return mean + 2.f * stddev; }
        [[nodiscard]] float adjustedMinPsize() const { return mean - 2.f * stddev; }
        [[nodiscard]] float adjustedMaxPsize() const { return mean + 2.f * stddev; }
    };

    std::array<PsizeInfo, cfg::MITypes::LEN_TYPE_NUM> psizeInfos;

    for (int heapIdx = 0; heapIdx < heaps.size(); heapIdx++) {
        auto& heap = heaps[heapIdx];
        _hp::MeanStddev psizeMeanStddev{};
        while (!heap.empty()) {
            const auto [grads, psize] = heap.top();
            heap.pop();
            psizeMeanStddev.update(psize);
        }

        psizeInfos[heapIdx] = {psizeMeanStddev.getMean(), psizeMeanStddev.getStddev()};
    }

    // heap adjust
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            // adjust patch size
            const int miType = miTypes.getMIType(row, col);
            const float psize = bridge.getInfo(row, col).getPatchsize();
            const auto& psizeInfo = psizeInfos[miType];
            if (psize > psizeInfo.maxPsize()) {
                bridge.getInfo(row, col).setPatchsize(psizeInfo.adjustedMaxPsize());
            } else if (psize < psizeInfo.minPsize()) {
                bridge.getInfo(row, col).setPatchsize(psizeInfo.adjustedMinPsize());
            }

            // adjust weight
            const auto& mi = mis_.getMI(row, col);
            const float weight = mi.grads + 0.01f;
            bridge.setWeight(row, col, weight);
        }
    }

    // neighbor adjust
    typename TBridge::TInfos rawInfos = bridge.getInfos();
    const auto& nearFocalLenTypePInfo = psizeInfos[arrange_.getNearFocalLenType()];
    const auto& farFocalLenTypePInfo = psizeInfos[arrange_.getNearFocalLenType() + 2 % cfg::MITypes::LEN_TYPE_NUM];
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int miType = miTypes.getMIType(row, col);
            if (miType != arrange_.getNearFocalLenType()) {
                continue;
            }

            const int offset = row * arrange_.getMIMaxCols() + col;

            using TNeighbors = NearNeighbors_<TArrange>;
            const auto neighbors = TNeighbors::fromArrangeAndIndex(arrange_, {col, row});

            const float psizeThre = nearFocalLenTypePInfo.mean + 1.5f * nearFocalLenTypePInfo.stddev;
            float neibPSizeSum = 0.f;
            int neibCount = 0;
            int satisfiedNeibCount = 0;
            for (const auto direction : TNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) {
                    continue;
                }

                const cv::Point neibIdx = neighbors.getNeighborIdx(direction);
                const int neibOffset = neibIdx.y * arrange_.getMIMaxCols() + neibIdx.x;
                const float neibPSize = rawInfos[neibOffset].getPatchsize();
                if (neibPSize > psizeThre) {
                    satisfiedNeibCount++;
                }

                neibPSizeSum += neibPSize;
                neibCount++;
            }

            const float avgNeibPSize = neibPSizeSum / neibCount;
            if (satisfiedNeibCount >= 5) {
                bridge.getInfo(offset).setPatchsize(avgNeibPSize);
            }
        }
    }

    rawInfos = bridge.getInfos();
    for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
            const int offset = row * arrange_.getMIMaxCols() + col;

            const int miType = miTypes.getMIType(row, col);
            if (miType == arrange_.getNearFocalLenType()) {
                continue;
            }

            using TNeighbors = NearNeighbors_<TArrange>;
            const auto neighbors = TNeighbors::fromArrangeAndIndex(arrange_, {col, row});

            const float psizeThre = farFocalLenTypePInfo.mean - farFocalLenTypePInfo.stddev;
            float neibPSizeSum = 0.f;
            int neibCount = 0;
            int satisfiedNeibCount = 0;
            for (const auto direction : TNeighbors::DIRECTIONS) {
                if (!neighbors.hasNeighbor(direction)) {
                    continue;
                }

                const cv::Point neibIdx = neighbors.getNeighborIdx(direction);
                const int neibMIType = miTypes.getMIType(neibIdx);
                if (neibMIType != arrange_.getNearFocalLenType()) {
                    continue;
                }

                const int neibOffset = neibIdx.y * arrange_.getMIMaxCols() + neibIdx.x;
                const float neibPSize = rawInfos[neibOffset].getPatchsize();
                if (neibPSize < psizeThre) {
                    satisfiedNeibCount++;
                }

                neibPSizeSum += neibPSize;
                neibCount++;
            }

            const float avgNeibPSize = neibPSizeSum / neibCount;
            if (satisfiedNeibCount >= 2) {
                bridge.getInfo(offset).setPatchsize(avgNeibPSize);
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

    auto prevMisRes = TMIBuffers::create(arrange);
    if (!prevMisRes) return std::unexpected{std::move(prevMisRes.error())};
    auto& prevMis = prevMisRes.value();

    std::vector<TPInfo> prevPatchInfos(arrange.getMIRows() * arrange.getMIMaxCols());

    auto paramsRes = TPsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), std::move(prevMis), std::move(prevPatchInfos), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::updateBridge(const cv::Mat& src, TBridge& bridge) noexcept {
    std::swap(mis_, prevMis_);
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
