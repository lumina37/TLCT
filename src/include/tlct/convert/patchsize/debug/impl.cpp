#include <bit>
#include <format>
#include <limits>
#include <queue>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/patchsize/ssim/functional.hpp"
#include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#include "tlct/convert/patchsize/ssim/params.hpp"
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
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(const TNeighbors& neighbors,
                                                        ssim::WrapSSIM& wrapAnchor) const noexcept {
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

        const ssim::MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        ssim::WrapSSIM wrapNeib{neibMI};

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
    using PsizeParams = ssim::PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const ssim::MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = prevPatchInfos_[offset].getPatchsize();

    bridge.getInfo(offset).setDhash(anchorMI.dhash);
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const uint16_t prevDhash = prevPatchInfos_[offset].getDhash();
        const uint16_t hashDist = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if (hashDist <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

    float bestPsize;
    ssim::WrapSSIM wrapAnchor{anchorMI};
    const cfg::MITypes mitypes{arrange_.isOutShift()};
    const int miType = mitypes.getMIType(index);
    if (arrange_.isMultiFocus() && miType == arrange_.getNearFocalLenType()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbors<FarNeighbors>(farNeighbors, wrapAnchor);
        bestPsize = farPsizeMetric.psize;
    } else {
        const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
        const PsizeMetric& nearPsizeMetric = estimateWithNeighbors<NearNeighbors>(nearNeighbors, wrapAnchor);
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
            if (satisfiedNeibCount >= 6) {
                bridge.setWeight(row, col, 0.01f);
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

}  // namespace tlct::_cvt::dbg
