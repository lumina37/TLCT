#include <cmath>
#include <limits>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/helper/std.hpp"

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

[[nodiscard]] float computeGrads(const cv::Mat& src) noexcept {
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
    const float grad = std::sqrt(sqrX * 0.5f + sqrY * 0.5f);

    return grad;
}

uint16_t dhash(const cv::Mat& src) {
    constexpr int THUMB_WIDTH = 4;
    constexpr int THUMB_SIZE = THUMB_WIDTH * (THUMB_WIDTH + 1);
    std::array<uint8_t, THUMB_SIZE> thumbBuffer;
    cv::Mat thumbnail(THUMB_WIDTH, THUMB_WIDTH + 1, CV_8UC1, thumbBuffer.data());
    cv::resize(src, thumbnail, thumbnail.size());

    uint16_t dhash = 0;
    uint16_t mask = 1;
    for (const int row : rgs::views::iota(0, THUMB_WIDTH)) {
        const auto prow = thumbnail.ptr<uint8_t>(row);
        for (const int col : rgs::views::iota(0, THUMB_WIDTH)) {
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
