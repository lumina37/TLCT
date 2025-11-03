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
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/impl.hpp"
#endif

namespace tlct::_cvt {

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& params) noexcept
    : arrange_(arrange), mis_(std::move(mis)), params_(params) {
    prevPatchRecords_.resize(arrange.getMIRows() * arrange.getMIMaxCols());
    patchRecords_.resize(prevPatchRecords_.size());

    if (arrange_.isMultiFocus()) {
        weights_.resize(prevPatchRecords_.size());
    }
}

template <cfg::concepts::CArrange TArrange>
auto PsizeImpl_<TArrange>::maxGradDirectionWithNearNeighbors(const NearNeighbors& neighbors) const noexcept ->
    typename NearNeighbors::Direction {
    float maxGrad = -1.f;
    typename NearNeighbors::Direction maxGradDirection{};
    for (const auto direction : {NearNeighbors::Direction::UPLEFT, NearNeighbors::Direction::UPRIGHT,
                                 NearNeighbors::Direction::DOWNLEFT, NearNeighbors::Direction::DOWNRIGHT}) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.grads.deg60 > maxGrad) {
            maxGrad = neibMI.grads.deg60;
            maxGradDirection = direction;
        }
    }

    for (const auto direction : {NearNeighbors::Direction::LEFT, NearNeighbors::Direction::RIGHT}) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.grads.deg0 > maxGrad) {
            maxGrad = neibMI.grads.deg0;
            maxGradDirection = direction;
        }
    }

    return maxGradDirection;
}

template <cfg::concepts::CArrange TArrange>
auto PsizeImpl_<TArrange>::maxGradDirectionWithFarNeighbors(const FarNeighbors& neighbors) const noexcept ->
    typename FarNeighbors::Direction {
    float maxGrad = -1.f;
    typename FarNeighbors::Direction maxGradDirection{};
    for (const auto direction : {FarNeighbors::Direction::UPLEFT, FarNeighbors::Direction::UPRIGHT,
                                 FarNeighbors::Direction::DOWNLEFT, FarNeighbors::Direction::DOWNRIGHT}) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.grads.deg30 > maxGrad) {
            maxGrad = neibMI.grads.deg30;
            maxGradDirection = direction;
        }
    }

    for (const auto direction : {FarNeighbors::Direction::UP, FarNeighbors::Direction::DOWN}) {
        if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
            continue;
        }

        const MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
        if (neibMI.grads.deg90 > maxGrad) {
            maxGrad = neibMI.grads.deg90;
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
        if constexpr (ENABLE_DEBUG) {
            metrics.push_back(diffRatio);
        }
        if (diffRatio < minDiffRatio) {
            minDiffRatio = diffRatio;
            bestPsize = psize;
        }
    }

    const float psize = (float)bestPsize / TNeighbors::INFLATE;
    const float metric = minDiffRatio;

    if constexpr (ENABLE_DEBUG) {
        const auto index = neighbors.getSelfIdx();
        const int offset = index.y * arrange_.getMIMaxCols() + index.x;
        if constexpr (std::is_same_v<TNeighbors, FarNeighbors>) {
            patchRecords_[offset].setFarMetrics(std::move(metrics));
        } else {
            patchRecords_[offset].setNearMetrics(std::move(metrics));
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

    if constexpr (ENABLE_DEBUG) {
        patchRecords_[offset] = {};
    }
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        // Early return if dhash is only slightly different
        const uint16_t prevDhash = prevPatchRecords_[offset].getDhash();
        const uint16_t dhashDiff = (uint16_t)std::popcount((uint16_t)(prevDhash ^ anchorMI.dhash));
        if constexpr (ENABLE_DEBUG) {
            patchRecords_[offset].setDhashDiff(dhashDiff);
        }
        patchRecords_[offset].setDhash(anchorMI.dhash);
        if (dhashDiff <= params_.psizeShortcutThreshold) {
            return prevPsize;
        }
    }

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    const auto nearDirection = maxGradDirectionWithNearNeighbors(nearNeighbors);

    const PsizeMetric& nearPsizeMetric = estimateWithNeighbors<NearNeighbors>(nearNeighbors, anchorMI, nearDirection);
    float minMetric = nearPsizeMetric.metric;
    float bestPsize = nearPsizeMetric.psize;

    if (arrange_.isMultiFocus()) {
        const FarNeighbors& farNeighbors = FarNeighbors::fromArrangeAndIndex(arrange_, index);
        const auto farDirection = maxGradDirectionWithFarNeighbors(farNeighbors);
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
            texMeanStddev.update(mi.grads.normed);
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
            const float currGrad = mi.grads.normed;

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
                neibGrads[(int)direction] = neibMI.grads.normed;
                neibPsizes[(int)direction] = getPatchsize(neibOffset);
            }

            const float normedGrad = (mi.grads.normed - texGradMean) / (texGradStddev * 2.0f);
            const float clippedGrad = _hp::clip(normedGrad, -1.0f, 1.0f);
            const float poweredGrad = clippedGrad * clippedGrad * clippedGrad;
            weights_[offset] = poweredGrad + 1.0f;

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
                weights_[offset] /= 2.0f;
                patchRecords_[offset].setPsize(std::reduce(neibPsizes.begin(), neibPsizes.end(), 0.f) /
                                               TNeighbors::DIRECTION_NUM);

                if constexpr (ENABLE_DEBUG) {
                    patchRecords_[offset].setIsBlurredFar(true);
                }

                continue;
            }

            // For blurred MI in near field.
            // These MI will have exactly 3 neighbor MIs that have clearer texture.
            // We should set their patch sizes to the average patch sizes of their clearer neighbor MIs.
            if (group0GtCount == 3 && group1GtCount == 0) {
                patchRecords_[offset].setPsize((neibPsizes[(int)Direction::UPLEFT] + neibPsizes[(int)Direction::RIGHT] +
                                                neibPsizes[(int)Direction::DOWNLEFT]) /
                                               3.f);
                if constexpr (ENABLE_DEBUG) {
                    patchRecords_[offset].setIsBlurredNear(true);
                }
            } else if (group0GtCount == 0 && group1GtCount == 3) {
                patchRecords_[offset].setPsize((neibPsizes[(int)Direction::UPRIGHT] + neibPsizes[(int)Direction::LEFT] +
                                                neibPsizes[(int)Direction::DOWNLEFT]) /
                                               3.f);
                if constexpr (ENABLE_DEBUG) {
                    patchRecords_[offset].setIsBlurredNear(true);
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

    auto paramsRes = PsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::dumpRecords(const fs::path& dumpTo) const noexcept {
    std::ofstream ofs{dumpTo, std::ios::binary};
    if (!ofs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}, iostate={}", dumpTo.string(), (int)ofs.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ofs.write((char*)patchRecords_.data(), patchRecords_.size() * sizeof(PatchRecord));
    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::update(const cv::Mat& src) noexcept {
    std::swap(prevPatchRecords_, patchRecords_);

    auto updateRes = mis_.update(src);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int idx = 0; idx < (int)patchRecords_.size(); idx++) {
        const int row = idx / arrange_.getMIMaxCols();
        const int col = idx % arrange_.getMIMaxCols();
        if (col >= arrange_.getMICols(row)) {
            continue;
        }
        const cv::Point index{col, row};
        const float psize = estimatePatchsize(index);
        patchRecords_[idx].setPsize(psize);
    }

    if (arrange_.isMultiFocus()) {
        adjustWgtsAndPsizesForMultiFocus();
    }

    return {};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::loadRecords(const fs::path& loadFrom) noexcept {
    std::ifstream ifs{loadFrom, std::ios::binary};
    if (!ifs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open file. path={}, iostate={}", loadFrom.string(), (int)ifs.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ifs.read((char*)patchRecords_.data(), patchRecords_.size() * sizeof(PatchRecord));
    return {};
}

template class PsizeImpl_<cfg::CornersArrange>;
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
