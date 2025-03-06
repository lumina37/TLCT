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
    cv::Mat srcY_, censusMap_, censusMask_;

private:
    mutable cv::Mat I1I2, mu1mu2, sigma12;
};

void WrapCensus::updateRoi(cv::Rect roi) noexcept {
    srcY_ = mi_.srcY(roi);
    censusMap_ = mi_.censusMap(roi);
    censusMask_ = mi_.censusMask(roi);
}

float WrapCensus::compare(const WrapCensus& rhs) const noexcept {
    uint64_t diffSum = 0;
    for (int row = 0; row < censusMap_.rows; row++) {
        const cv::Vec3b* pLhsMap = censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pLhsMask = censusMask_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMap = rhs.censusMap_.ptr<cv::Vec3b>(row);
        const cv::Vec3b* pRhsMask = rhs.censusMask_.ptr<cv::Vec3b>(row);
        for (int col = 0; col < censusMap_.cols; col++) {
            for (int vecIdx = 0; vecIdx < 3; vecIdx++) {
                const uint8_t mask = (*pLhsMask)[vecIdx] & (*pRhsMask)[vecIdx];
                const uint8_t diff = (*pLhsMap)[vecIdx] ^ (*pRhsMap)[vecIdx];
                const uint8_t maskedDiff = mask & diff;
                diffSum += std::popcount(maskedDiff);
            }
            pLhsMap++;
            pLhsMask++;
            pRhsMap++;
            pRhsMask++;
        }
    }

    const float avgDiff = (float)diffSum / (float)(censusMap_.total() * 3);
    return avgDiff;
}

}  // namespace tlct::_cvt
