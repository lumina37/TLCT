#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ranges>
#include <span>

#include <opencv2/imgproc.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] cv::Mat circleWithFadeoutBorder(const int diameter, const float borderWidthFactor) noexcept {
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

[[nodiscard]] Grads computeGrads(const cv::Mat& src) noexcept {
    cv::Mat edges;
    const float pixCount = (float)src.total();

    cv::Scharr(src, edges, CV_32F, 1, 0);
    edges = cv::abs(edges);
    const float gradX = (float)cv::sum(edges)[0] / pixCount;
    cv::Scharr(src, edges, CV_32F, 0, 1);
    edges = cv::abs(edges);
    const float gradY = (float)cv::sum(edges)[0] / pixCount;

    const float sqrX = gradX * gradX;
    const float sqrY = gradY * gradY;
    const float gradDeg30 = std::sqrt(sqrX * 0.75f + sqrY * 0.25f);
    const float gradDeg60 = std::sqrt(sqrX * 0.25f + sqrY * 0.75f);
    const float gradNormed = std::sqrt(sqrX * 0.5f + sqrY * 0.5f);

    return {gradX, gradDeg30, gradDeg60, sqrY, gradNormed};
}

[[nodiscard]] TLCT_API int pickByFWHM(const std::span<float> arr) noexcept {
    const float maxVal = *rgs::max_element(arr);
    const auto& correctedArr = arr | rgs::views::transform([maxVal](const float v) { return maxVal - v; });

    float maxArea = -1.f;
    int maxAreaIdx = 0;
    for (const int idx : rgs::views::iota(0, (int)arr.size())) {
        const float currVal = correctedArr[idx];
        const float threVal = currVal * 0.5f;

        float area = currVal;
        for (int leftIdx = idx - 1; leftIdx >= 0; leftIdx--) {
            const float leftVal = correctedArr[leftIdx];
            if (leftVal > currVal || leftVal < threVal) {
                break;
            }
            area += leftVal;
        }

        for (int rightIdx = idx + 1; rightIdx < arr.size(); rightIdx++) {
            const float rightVal = correctedArr[rightIdx];
            if (rightVal > currVal || rightVal < threVal) {
                break;
            }
            area += rightVal;
        }

        if (area > maxArea) {
            maxArea = area;
            maxAreaIdx = idx;
        }
    }

    return maxAreaIdx;
}

uint16_t dhash(const cv::Mat& src) {
    constexpr int thumbnail_width = 4;
    constexpr int thumbnail_size = thumbnail_width * (thumbnail_width + 1);
    std::array<uint8_t, thumbnail_size> thumbnail_buffer;
    cv::Mat thumbnail(thumbnail_width, thumbnail_width, CV_8UC1, thumbnail_buffer.data());
    cv::resize(src, thumbnail, {thumbnail_width + 1, thumbnail_width});

    uint16_t dhash = 0;
    uint16_t mask = 1;
    for (const int row : rgs::views::iota(0, thumbnail_width)) {
        const auto prow = thumbnail.ptr<uint8_t>(row);
        for (const int col : rgs::views::iota(0, thumbnail_width)) {
            const uint8_t currVal = prow[col];
            const uint8_t nextVal = prow[col + 1];
            if (nextVal > currVal) {
                dhash |= mask;
            }
            mask <<= 1;
        }
    }

    return dhash;
}

}  // namespace tlct::_cvt
