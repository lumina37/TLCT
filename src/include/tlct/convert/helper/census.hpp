#pragma once

#include <bit>
#include <cstdint>

#include <opencv2/imgproc.hpp>

#include "tlct/convert/helper/mibuffer.hpp"

namespace tlct::_cvt {

class WrapCensus {
public:
    // Constructor
    WrapCensus() = delete;
    inline explicit WrapCensus(const MIBuffer& mi) noexcept : mi_(mi) {};
    inline WrapCensus(const WrapCensus& rhs) = default;
    WrapCensus& operator=(const WrapCensus& rhs) = delete;
    WrapCensus(WrapCensus&& rhs) noexcept = default;
    WrapCensus& operator=(WrapCensus&& rhs) noexcept = delete;

    // Const methods
    [[nodiscard]] inline float compare(const WrapCensus& rhs) const noexcept;

    // Non-const methods
    inline void updateRoi(cv::Rect roi) noexcept;

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
    for (int row = 0; row < censusMap_.rows; row++) {
        const cv::Vec3b* pLhsMap = censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pLhsMask = censusMask_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMap = rhs.censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMask = rhs.censusMask_.ptr<cv::Vec3b>(row);
        for (int col = 0; col < censusMap_.cols; col++) {
            for (int byteId = 0; byteId < BYTE_COUNT; byteId++) {
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
