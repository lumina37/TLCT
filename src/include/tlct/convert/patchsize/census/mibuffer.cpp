#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <memory>
#include <numbers>
#include <ranges>
#include <utility>
#include <vector>

#include <opencv2/imgproc.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/patchsize/census/functional.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/census/mibuffer.hpp"
#endif

namespace tlct::_cvt::census {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::MIBuffers_(TArrange&& arrange, Params&& params, std::vector<MIBuffer>&& miBuffers,
                                 std::unique_ptr<std::byte[]>&& pBuffer) noexcept
    : arrange_(std::move(arrange)),
      params_(std::move(params)),
      miBuffers_(std::move(miBuffers)),
      pBuffer_(std::move(pBuffer)) {}

template <cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::Params::Params(const TArrange& arrange) noexcept {
    censusDiameter_ = arrange.getDiameter() * CENSUS_SAFE_RATIO;
    const int iCensusDiameter = _hp::iround(censusDiameter_);
    alignedMatSizeC3_ = _hp::alignUp<SIMD_FETCH_SIZE>(iCensusDiameter * iCensusDiameter * 3);
    alignedMISize_ = alignedMatSizeC3_ * MIBuffer::C3_COUNT;
    miMaxCols_ = arrange.getMIMaxCols();
    miNum_ = miMaxCols_ * arrange.getMIRows();
    bufferSize_ = miNum_ * alignedMISize_;
}

template <cfg::concepts::CArrange TArrange>
auto MIBuffers_<TArrange>::create(const TArrange& arrange) noexcept -> std::expected<MIBuffers_, Error> {
    auto copiedArrange = arrange;
    Params params{arrange};
    try {
        std::vector<MIBuffer> miBuffers(params.miNum_);
        auto pBuffer = std::make_unique_for_overwrite<std::byte[]>(params.bufferSize_ + Params::SIMD_FETCH_SIZE);
        return MIBuffers_{std::move(copiedArrange), std::move(params), std::move(miBuffers), std::move(pBuffer)};
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ECate::eSys, ECode::eOutOfMemory}};
    }
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> MIBuffers_<TArrange>::update(const cv::Mat& src) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    if (src.type() != CV_8UC1) [[unlikely]] {
        auto errMsg = std::format("MIBuffers::update expect CV_8UC1, got {}", src.type());
        return std::unexpected{Error{ECate::eTLCT, ECode::eUnexValue, std::move(errMsg)}};
    }

    const int iCensusDiameter = _hp::iround(params_.censusDiameter_);
    const float censusRadius = params_.censusDiameter_ / 2.f;
    const int iCensusRadius = _hp::iround(params_.censusDiameter_ / 2.f);
    const cv::Mat srcCircleMask = cv::Mat::zeros(iCensusDiameter, iCensusDiameter, CV_8UC1);
    cv::circle(srcCircleMask, {iCensusRadius, iCensusRadius}, iCensusRadius, cv::Scalar::all(0xff), cv::FILLED);

    const cv::Rect dhashRoi =
        getRoiByCenter({censusRadius, censusRadius}, params_.censusDiameter_ / std::numbers::sqrt2_v<float>);

    uint8_t* bufBase = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)pBuffer_.get());
#pragma omp parallel for
    for (int idx = 0; idx < params_.miNum_; idx++) {
        const int rowMIIdx = idx / params_.miMaxCols_;
        const int colMIIdx = idx % params_.miMaxCols_;
        if (colMIIdx >= arrange_.getMICols(rowMIIdx)) {
            continue;
        }

        auto miBufIterator = miBuffers_.begin() + idx;
        cv::Mat tmpY = cv::Mat(iCensusDiameter, iCensusDiameter, CV_8UC1);

        const cv::Point2f& miCenter = arrange_.getMICenter(rowMIIdx, colMIIdx);
        const cv::Rect miRoi = getRoiByCenter(miCenter, params_.censusDiameter_);

        uint8_t* matBufCursor = bufBase + idx * params_.alignedMISize_;

        const cv::Mat& srcY = src(miRoi);
        srcY.copyTo(tmpY);
        cv::Mat centralY = tmpY(dhashRoi);

        cv::Mat censusMap = cv::Mat(iCensusDiameter, iCensusDiameter, CV_8UC3, matBufCursor);
        matBufCursor += params_.alignedMatSizeC3_;
        cv::Mat censusMask = cv::Mat(iCensusDiameter, iCensusDiameter, CV_8UC3, matBufCursor);
        censusTransform5x5(tmpY, srcCircleMask, censusMap, censusMask);

        miBufIterator->censusMap = std::move(censusMap);
        miBufIterator->censusMask = std::move(censusMask);

        const float grads = computeGrads(centralY);
        miBufIterator->grads = grads;

        miBufIterator->dhash = dhash(centralY);
    }

    return {};
}

[[nodiscard]] float compare(const MIBuffer& lhsMI, const MIBuffer& rhsMI, cv::Point2f offset) noexcept {
    assert(lhsMI.censusMap.size() == rhsMI.censusMap.size());

    const std::array ranges{cv::Range{0, lhsMI.censusMap.rows}, cv::Range{0, lhsMI.censusMap.cols}};

    const auto applyOffset = [&ranges](cv::Point offset) {
        auto [rowRange, colRange] = ranges;
        if (offset.y > 0) {
            rowRange.start += offset.y;
        } else {
            rowRange.end += offset.y;
        }
        if (offset.x > 0) {
            colRange.start += offset.x;
        } else {
            colRange.end += offset.x;
        }
        return std::array{rowRange, colRange};
    };

    const auto lhsRanges = applyOffset(-offset);
    const auto rhsRanges = applyOffset(offset);

    const auto& lhsCensusMap = lhsMI.censusMap(lhsRanges.data());
    const auto& lhsCensusMask = lhsMI.censusMask(lhsRanges.data());

    const auto& rhsCensusMap = rhsMI.censusMap(rhsRanges.data());
    const auto& rhsCensusMask = rhsMI.censusMask(rhsRanges.data());

    uint64_t maskBitCount = 0;
    uint64_t diffBitCount = 0;
    for (const int row : rgs::views::iota(0, lhsRanges[0].size())) {
        const cv::Vec3b* pLhsMap = lhsCensusMap.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pLhsMask = lhsCensusMask.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMap = rhsCensusMap.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMask = rhsCensusMask.ptr<cv::Vec3b>(row);
        for ([[maybe_unused]] const int _ : rgs::views::iota(0, lhsRanges[1].size())) {
            constexpr int BYTE_COUNT = sizeof(cv::Vec3b) / sizeof(uint8_t);
            for (const int byteId : rgs::views::iota(0, BYTE_COUNT)) {
                const uint8_t diff = (*pLhsMap)[byteId] ^ (*pRhsMap)[byteId];
                const uint8_t mask = (*pLhsMask)[byteId] & (*pRhsMask)[byteId];
                const uint8_t maskedDiff = mask & diff;
                maskBitCount += std::popcount(mask);
                diffBitCount += std::popcount(maskedDiff);
            }
            pLhsMap++;
            pLhsMask++;
            pRhsMap++;
            pRhsMask++;
        }
    }

    const float diffRatio = (float)diffBitCount / (float)maskBitCount;
    return diffRatio;
}

template class MIBuffers_<cfg::CornersArrange>;
template class MIBuffers_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::census
