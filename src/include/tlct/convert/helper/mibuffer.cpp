#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <numbers>
#include <ranges>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/census.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/helper/constexpr/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/mibuffer.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::Params::Params(const TArrange& arrange) noexcept {
    iDiameter_ = _hp::iround(arrange.getDiameter());
    alignedMatSizeC1_ = _hp::alignUp<SIMD_FETCH_SIZE>(iDiameter_ * iDiameter_);
    alignedMatSizeC3_ = _hp::alignUp<SIMD_FETCH_SIZE>(iDiameter_ * iDiameter_ * 3);
    alignedMISize_ = alignedMatSizeC1_ * MIBuffer::C1_COUNT + alignedMatSizeC3_ * MIBuffer::C3_COUNT;
    miMaxCols_ = arrange.getMIMaxCols();
    miNum_ = miMaxCols_ * arrange.getMIRows();
    bufferSize_ = miNum_ * alignedMISize_;
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::MIBuffers_(const TArrange& arrange) : arrange_(arrange), params_(arrange) {
    miBuffers_.resize(params_.miNum_);
    buffer_ = std::malloc(params_.bufferSize_ + Params::SIMD_FETCH_SIZE);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange> MIBuffers_<TArrange>::fromArrange(const TArrange& arrange) {
    return MIBuffers_(arrange);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>& MIBuffers_<TArrange>::update(const cv::Mat& src) {
    assert(src.type() == CV_8UC1);

    const int iDiameter = _hp::iround(arrange_.getDiameter());
    const float radius = arrange_.getRadius();
    const int iRadius = _hp::iround(arrange_.getRadius());
    const int iCensusRadius = _hp::iround(arrange_.getRadius() * 0.75f);
    const cv::Mat srcCircleMask = cv::Mat::zeros(iDiameter, iDiameter, CV_8UC1);
    cv::circle(srcCircleMask, {iRadius, iRadius}, iCensusRadius, cv::Scalar::all(0xff), cv::FILLED);

    const float textureRoiWidth = arrange_.getDiameter() / std::numbers::sqrt2_v<float> * SAFE_RATIO;
    const cv::Rect textureRoi = getRoiByCenter({radius, radius}, textureRoiWidth);

    uint8_t* bufBase = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)buffer_);
    size_t rowBufStep = params_.miMaxCols_ * params_.alignedMISize_;
#pragma omp parallel for
    for (int rowMIIdx = 0; rowMIIdx < arrange_.getMIRows(); rowMIIdx++) {
        uint8_t* colBufCursor = bufBase + rowMIIdx * rowBufStep;
        auto miBufIterator = miBuffers_.begin() + rowMIIdx * arrange_.getMIMaxCols();

        const int miCols = arrange_.getMICols(rowMIIdx);
        for (int colMIIdx = 0; colMIIdx < miCols; colMIIdx++) {
            const cv::Point2f& miCenter = arrange_.getMICenter(rowMIIdx, colMIIdx);
            const cv::Rect miRoi = getRoiByCenter(miCenter, arrange_.getDiameter());

            uint8_t* matBufCursor = colBufCursor;

            const cv::Mat& srcY = src(miRoi);
            cv::Mat dstY = cv::Mat(params_.iDiameter_, params_.iDiameter_, CV_8UC1, matBufCursor);
            srcY.copyTo(dstY);
            matBufCursor += params_.alignedMatSizeC1_;

            cv::Mat censusMap = cv::Mat(params_.iDiameter_, params_.iDiameter_, CV_8UC3, matBufCursor);
            matBufCursor += params_.alignedMatSizeC3_;
            cv::Mat censusMask = cv::Mat(params_.iDiameter_, params_.iDiameter_, CV_8UC3, matBufCursor);
            matBufCursor += params_.alignedMatSizeC3_;
            censusTransform5x5(dstY, srcCircleMask, censusMap, censusMask);

            const cv::Mat texture = dstY(textureRoi);
            const float intensity = textureIntensity(texture);

            *miBufIterator = {std::move(dstY), std::move(censusMap), std::move(censusMask), intensity};
            miBufIterator++;
            colBufCursor += params_.alignedMISize_;
        }
    }

    return *this;
}

[[nodiscard]] float censusCompare(const MIBuffer& lhsMI, const MIBuffer& rhsMI, cv::Point2f offset) noexcept {
    assert(lhsMI.srcY.size() == rhsMI.srcY.size());

    const std::array ranges{cv::Range{0, lhsMI.srcY.rows}, cv::Range{0, lhsMI.srcY.cols}};

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

template class MIBuffers_<_cfg::CornersArrange>;
template class MIBuffers_<_cfg::OffsetArrange>;

}  // namespace tlct::_cvt
