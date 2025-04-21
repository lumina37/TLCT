#include <cassert>
#include <cstdint>
#include <ranges>

#include <opencv2/core.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/census.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

void censusTransform5x5(const cv::Mat& src, const cv::Mat& srcMask, cv::Mat& censusMap, cv::Mat& censusMask) noexcept {
    assert(src.type() == CV_8UC1);
    assert(srcMask.type() == CV_8UC1);
    assert(censusMap.elemSize() == 3);
    assert(censusMask.elemSize() == 3);

    const auto isInRange = [&src](const int row, const int col) {
        if (row < 0 || row >= src.rows) [[unlikely]]
            return false;
        if (col < 0 || col >= src.cols) [[unlikely]]
            return false;
        return true;
    };

    for (const int row : rgs::views::iota(0, src.rows)) {
        cv::Vec3b* pCsMap = censusMap.ptr<cv::Vec3b>(row);
        cv::Vec3b* pCsMask = censusMask.ptr<cv::Vec3b>(row);
        for (const int col : rgs::views::iota(0, src.cols)) {
            // For each pixel
            constexpr int WINDOW = 5;
            constexpr int HALF_WINDOW = WINDOW / 2;
            // Deal with the window
            int winPixCount = 0;
            const uint8_t centralPix = src.at<uint8_t>(row, col);
            for (int winRow = -HALF_WINDOW; winRow <= HALF_WINDOW; winRow++) {
                for (int winCol = -HALF_WINDOW; winCol <= HALF_WINDOW; winCol++) {
                    if (winRow == 0 && winCol == 0) [[unlikely]]
                        continue;  // skip the central pixel

                    const int byteId = winPixCount / 8;
                    const int bitOffset = winPixCount % 8;
                    constexpr uint8_t one = 1;
                    if (!isInRange(row + winRow, col + winCol)) [[unlikely]] {
                        (*pCsMask)[byteId] &= ~(one << bitOffset);  // bit set pMask[vecIdx][vecShift] = 0
                    } else {
                        const uint8_t srcMaskVal = srcMask.at<uint8_t>(row + winRow, col + winCol);
                        if (srcMaskVal == 0) [[unlikely]] {
                            (*pCsMask)[byteId] &= ~(one << bitOffset);  // bit set pMask[vecIdx][vecShift] = 0
                        } else {
                            const uint8_t srcPix = src.at<uint8_t>(row + winRow, col + winCol);
                            (*pCsMask)[byteId] |= (one << bitOffset);  // bit set pMask[vecIdx][vecShift] = 1
                            if (srcPix > centralPix) {
                                (*pCsMap)[byteId] |= (one << bitOffset);  // bit set pMap[vecIdx][vecShift] = 1
                            } else {
                                (*pCsMap)[byteId] &= ~(one << bitOffset);  // bit set pMap[vecIdx][vecShift] = 1
                            }
                        }
                    }
                    // bump for each pixel in window
                    winPixCount++;
                }
            }
            // bump for each pixel in src
            pCsMap++;
            pCsMask++;
        }
    }
}

}  // namespace tlct::_cvt
