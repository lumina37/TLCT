#include <bit>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <numeric>
#include <ranges>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/census/mibuffer.hpp"
#include "tlct/convert/patchsize/neighbors.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/census/impl.hpp"
#endif

namespace tlct::_cvt::census {

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TPatchInfoVec&& prevPatchInfoVec,
                                 TPatchInfos&& patchInfos, const TPsizeParams& params) noexcept
    : arrange_(arrange),
      mis_(std::move(mis)),
      prevPatchInfoVec_(std::move(prevPatchInfoVec)),
      patchInfos_(std::move(patchInfos)),
      params_(params) {}

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
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(const TNeighbors& neighbors, const MIBuffer& anchorMI,
                                                        typename TNeighbors::Direction direction) noexcept {
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
            patchInfos_.getInfo(offset).getPDebugInfo()->farMetrics = std::move(metrics);
        } else {
            patchInfos_.getInfo(offset).getPDebugInfo()->nearMetrics = std::move(metrics);
        }
    }

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(cv::Point index) noexcept {
    using PsizeParams = PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = getPrevPatchsize(offset);

    if constexpr (DEBUG_ENABLED) {
        *patchInfos_.getInfo(offset).getPDebugInfo() = {};
    }
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        // Early return if dhash is only slightly different
        const uint16_t prevDhash = prevPatchInfoVec_[offset].getDhash();
        const uint16_t dhashDiff = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if constexpr (DEBUG_ENABLED) {
            patchInfos_.getInfo(offset).getPDebugInfo()->dhashDiff = dhashDiff;
        }
        patchInfos_.getInfo(offset).setDhash(anchorMI.dhash);
        if (dhashDiff <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    const auto nearDirection = maxGradDirection(nearNeighbors);

    const PsizeMetric& nearPsizeMetric = estimateWithNeighbors<NearNeighbors>(nearNeighbors, anchorMI, nearDirection);
    const float minMetric = nearPsizeMetric.metric;
    float bestPsize = nearPsizeMetric.psize;

    if (arrange_.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const auto farDirection = maxGradDirection(farNeighbors);
        const PsizeMetric& farPsizeMetric = estimateWithNeighbors<FarNeighbors>(farNeighbors, anchorMI, farDirection);
        if (farPsizeMetric.metric < minMetric) {
            bestPsize = farPsizeMetric.psize;
        }
    }

    return bestPsize;
}

template <cfg::concepts::CArrange TArrange>
void PsizeImpl_<TArrange>::adjustWgtsAndPsizesForMultiFocus() noexcept {
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
                neibPsizes[(int)direction] = patchInfos_.getInfo(neibOffset).getPatchsize();
            }

            const float normedGrad = (mi.grads - texGradMean) / (texGradStddev * 2.0f);
            const float clippedGrad = _hp::clip(normedGrad, -1.0f, 1.0f);
            const float poweredGrad = clippedGrad * clippedGrad * clippedGrad;
            patchInfos_.setWeight(offset, poweredGrad + 1.0f);

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
                const float newWeight = patchInfos_.getWeight(offset) / 2.f;
                patchInfos_.setWeight(offset, newWeight);
                const float psize = std::reduce(neibPsizes.begin(), neibPsizes.end(), 0.f) / TNeighbors::DIRECTION_NUM;
                patchInfos_.getInfo(offset).setPatchsize(psize);

                if constexpr (DEBUG_ENABLED) {
                    patchInfos_.getInfo(offset).getPDebugInfo()->isBlurredFar = true;
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
                patchInfos_.getInfo(offset).setPatchsize(psize);
                if constexpr (DEBUG_ENABLED) {
                    patchInfos_.getInfo(offset).getPDebugInfo()->isBlurredNear = true;
                }
            } else if (group0GtCount == 0 && group1GtCount == 3) {
                const float psize = (neibPsizes[(int)Direction::UPRIGHT] + neibPsizes[(int)Direction::LEFT] +
                                     neibPsizes[(int)Direction::DOWNLEFT]) /
                                    3.f;
                patchInfos_.getInfo(offset).setPatchsize(psize);
                if constexpr (DEBUG_ENABLED) {
                    patchInfos_.getInfo(offset).getPDebugInfo()->isBlurredNear = true;
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

    std::vector<TPatchInfo> prevPatchInfoVec(arrange.getMIRows() * arrange.getMIMaxCols());

    auto patchInfosRes = TPatchInfos::create(arrange);
    if (!patchInfosRes) return std::unexpected{std::move(patchInfosRes.error())};
    auto& patchInfos = patchInfosRes.value();

    auto paramsRes = TPsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), std::move(prevPatchInfoVec), std::move(patchInfos), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::update(const cv::Mat& src) noexcept {
    patchInfos_.swapInfos(prevPatchInfoVec_);

    auto updateRes = mis_.update(src);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int idx = 0; idx < (int)prevPatchInfoVec_.size(); idx++) {
        const int row = idx / arrange_.getMIMaxCols();
        const int col = idx % arrange_.getMIMaxCols();
        if (col >= arrange_.getMICols(row)) {
            continue;
        }
        const cv::Point index{col, row};
        const float psize = estimatePatchsize(index);
        patchInfos_.getInfo(idx).setPatchsize(psize);
    }

    if (arrange_.isMultiFocus()) {
        adjustWgtsAndPsizesForMultiFocus();
    }

    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::dumpInfos(const fs::path& dumpTo) const noexcept {
    std::ofstream ofs{dumpTo, std::ios::binary};
    if (!ofs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}", dumpTo.string());
        return std::unexpected{Error{ECate::eSys, ofs.rdstate(), std::move(errMsg)}};
    }

    const auto& patchInfoVec = patchInfos_.getInfoVec();
    ofs.write((char*)patchInfoVec.data(), patchInfoVec.size() * sizeof(patchInfoVec[0]));
    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::loadInfos(const fs::path& loadFrom) noexcept {
    std::ifstream ifs{loadFrom, std::ios::binary};
    if (!ifs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}", loadFrom.string());
        return std::unexpected{Error{ECate::eSys, ifs.rdstate(), std::move(errMsg)}};
    }

    auto& patchInfoVec = patchInfos_.getInfoVec();
    ifs.read((char*)patchInfoVec.data(), patchInfoVec.size() * sizeof(patchInfoVec[0]));
    return {};
}

template class PsizeImpl_<cfg::CornersArrange>;
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::census
