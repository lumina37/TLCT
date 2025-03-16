#pragma once

#include <bit>
#include <cstdint>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/convert/helper/mibuffer.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

class WrapCensus {
public:
    // Constructor
    WrapCensus() = delete;
    explicit WrapCensus(const MIBuffer& mi) noexcept : mi(mi) {};
    WrapCensus(const WrapCensus& rhs) = default;
    WrapCensus& operator=(const WrapCensus& rhs) = delete;
    WrapCensus(WrapCensus&& rhs) noexcept = default;
    WrapCensus& operator=(WrapCensus&& rhs) noexcept = delete;

    // Const methods
    [[nodiscard]] float compare(const WrapCensus& rhs, cv::Point2f offset) const noexcept;

    const MIBuffer& mi;
};

float WrapCensus::compare(const WrapCensus& rhs, cv::Point2f offset) const noexcept {
    const std::array ranges{cv::Range{0, mi.srcY.rows}, cv::Range{0, mi.srcY.cols}};

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

    const auto& lhsCensusMap = mi.censusMap(lhsRanges.data());
    const auto& lhsCensusMask = mi.censusMask(lhsRanges.data());

    const auto& rhsCensusMap = rhs.mi.censusMap(rhsRanges.data());
    const auto& rhsCensusMask = rhs.mi.censusMask(rhsRanges.data());

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

    const float diffRatio = (float)diffBitCount / (float)(maskBitCount);
    return diffRatio;
}

}  // namespace tlct::_cvt
