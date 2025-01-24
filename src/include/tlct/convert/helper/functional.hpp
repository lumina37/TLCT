#pragma once

#include <array>
#include <bit>
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
    cv::Sobel(src, edges, -1, 1, 0);
    edges = cv::abs(edges);
    intensity += (float)cv::sum(edges)[0];
    cv::Sobel(src, edges, -1, 0, 1);
    edges = cv::abs(edges);
    intensity += (float)cv::sum(edges)[0];
    intensity /= edges.size().area();
    return intensity;
}

[[nodiscard]] static inline uint64_t dhash(const cv::Mat& src) {
    constexpr int thumbnailHeight = 8;
    constexpr int thumbnailWidth = thumbnailHeight + 1;
    constexpr int thumbnailSize = thumbnailHeight * thumbnailWidth;
    std::array<float, thumbnailSize> thumbnailBuffer;
    cv::Mat thumbnail(thumbnailHeight, thumbnailWidth, CV_32FC1, thumbnailBuffer.data());
    cv::resize(src, thumbnail, {thumbnailWidth, thumbnailHeight}, 0., 0., cv::INTER_LINEAR_EXACT);

    uint64_t dhash = 0;
    const uint64_t u64max = std::numeric_limits<uint64_t>::max();
    uint64_t mask = u64max ^ (u64max >> 1);
    for (const int row : rgs::views::iota(0, thumbnailHeight)) {
        float* prow = thumbnail.ptr<float>(row);
        for ([[maybe_unused]] const int _ : rgs::views::iota(0, thumbnailWidth)) {
            const bool flag = *(prow + 1) > *prow;
            dhash |= flag * mask;
            mask >>= 1;
            prow++;
        }
    }

    return dhash;
}

[[nodiscard]] static inline int L1Dist(uint64_t lhs, uint64_t rhs) { return std::popcount(lhs ^ rhs); }

static inline void blurInto(const cv::Mat& src, cv::Mat& dst) { cv::GaussianBlur(src, dst, {11, 11}, 1.5); }

}  // namespace tlct::_cvt
