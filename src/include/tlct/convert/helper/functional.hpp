#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <ranges>

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] static inline cv::Mat circleWithFadeoutBorder(const int diameter, const float borderWidthFactor) {
    cv::Mat rect = cv::Mat::zeros({diameter, diameter}, CV_32FC1);
    const float radius = (float)diameter / 2.f;
    const float heap = borderWidthFactor > 0.f ? 1.f + 1.f / borderWidthFactor * (1.f - borderWidthFactor)
                                               : std::numeric_limits<float>::max();

    for (const int row : rgs::views::iota(0, diameter)) {
        float* prow = rect.ptr<float>(row);
        for (const int col : rgs::views::iota(0, diameter)) {
            const float xdist = radius - (float)row;
            const float ydist = radius - (float)col;
            const float dist = std::sqrt(xdist * xdist + ydist * ydist);
            const float pix = std::max(0.f, std::min(1.f, (1.f - dist / radius) * heap));
            *prow = pix;
            prow++;
        }
    }

    return rect;
}

[[nodiscard]] static inline float textureIntensity(const cv::Mat& src) {
    cv::Mat edges;
    float intensity = 0.0;
    cv::Sobel(src, edges, CV_32F, 1, 0);
    edges = cv::abs(edges);
    intensity += (float)cv::sum(edges)[0];
    cv::Sobel(src, edges, CV_32F, 0, 1);
    edges = cv::abs(edges);
    intensity += (float)cv::sum(edges)[0];
    intensity /= (float)edges.size().area();
    return intensity;
}

static inline void censusTransform5x5(const cv::Mat& src, const cv::Mat& srcMask, cv::Mat& censusMap,
                                      cv::Mat& censusMask) {
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
                    if (winRow == 0 && winCol == 0) continue;  // skip the central pixel

                    const int byteId = winPixCount / 8;
                    const int bitOffset = winPixCount % 8;
                    constexpr uint8_t one = 1;
                    if (!isInRange(row + winRow, col + winCol)) {
                        (*pCsMask)[byteId] &= ~(one << bitOffset);  // bit set pMask[vecIdx][vecShift] = 0
                    } else {
                        const uint8_t srcMaskVal = srcMask.at<uint8_t>(row + winRow, col + winCol);
                        if (srcMaskVal == 0) {
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
