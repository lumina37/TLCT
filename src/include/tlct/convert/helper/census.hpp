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
    explicit WrapCensus(const MIBuffer& mi) noexcept : mi_(mi) {};
    WrapCensus(const WrapCensus& rhs) = default;
    WrapCensus& operator=(const WrapCensus& rhs) = delete;
    WrapCensus(WrapCensus&& rhs) noexcept = default;
    WrapCensus& operator=(WrapCensus&& rhs) noexcept = delete;

    // Const methods
    [[nodiscard]] float compare(const WrapCensus& rhs) const noexcept;

    // Non-const methods
    void updateRoi(cv::Rect roi) noexcept;

    const MIBuffer& mi_;
    cv::Mat srcY_;

private:
    cv::Mat censusMap_, censusMask_;
};

void WrapCensus::updateRoi(cv::Rect roi) noexcept {
    srcY_ = mi_.srcY(roi);
    censusMap_ = mi_.censusMap(roi);
    censusMask_ = mi_.censusMask(roi);
}

float WrapCensus::compare(const WrapCensus& rhs) const noexcept {
    constexpr int BYTE_COUNT = sizeof(cv::Vec3b) / sizeof(uint8_t);

    uint64_t maskBitCount = 0;
    uint64_t diffBitCount = 0;
    for (const int row : rgs::views::iota(0, censusMap_.rows)) {
        const cv::Vec3b* pLhsMap = censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pLhsMask = censusMask_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMap = rhs.censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMask = rhs.censusMask_.ptr<cv::Vec3b>(row);
        for ([[maybe_unused]] const int _ : rgs::views::iota(0, censusMap_.cols)) {
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
