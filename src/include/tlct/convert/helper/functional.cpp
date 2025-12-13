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

cv::Mat circleWithFadeoutBorder(const int diameter, const float fadeBegin, const float fadeEnd) noexcept {
    cv::Mat rect = cv::Mat::zeros({diameter, diameter}, CV_32FC1);
    const float radius = (float)diameter / 2.f;

    for (const int row : rgs::views::iota(0, diameter)) {
        float* prow = rect.ptr<float>(row);
        for (const int col : rgs::views::iota(0, diameter)) {
            const float xdist = radius - (float)row;
            const float ydist = radius - (float)col;
            const float dist = std::sqrt(xdist * xdist + ydist * ydist);

            const float ratio = dist / radius;
            float pix;
            if (ratio <= fadeBegin) {
                pix = 1.f;
            } else if (ratio >= fadeEnd) {
                pix = 0.f;
            } else {
                pix = 1.f - (ratio - fadeBegin) / (fadeEnd - fadeBegin);
            }

            *prow = pix;
            prow++;
        }
    }

    return rect;
}

float computeGrads(const cv::Mat& src) noexcept {
    cv::Mat edges;
    const float pixCount = (float)src.total();

    float grads = 0.0;

    cv::Sobel(src, edges, CV_16S, 1, 0);
    edges = cv::abs(edges);
    grads += (float)cv::sum(edges)[0];

    cv::Sobel(src, edges, CV_16S, 0, 1);
    edges = cv::abs(edges);
    grads += (float)cv::sum(edges)[0];

    grads /= pixCount;
    return grads;
}

void computeGradsMap(const cv::Mat& src, cv::Mat& dst) noexcept {
    cv::Mat grads = cv::Mat::zeros(src.size(), src.type());
    cv::Mat edges(src.size(), src.type());

    cv::Sobel(src, edges, src.type(), 1, 0);
    edges = cv::abs(edges);
    grads += edges;

    cv::Sobel(src, edges, src.type(), 0, 1);
    edges = cv::abs(edges);
    grads += edges;

    constexpr int kSize = 3;
    constexpr float sigma = 1.0f;
    cv::GaussianBlur(grads, dst, {kSize, kSize}, sigma);
}

}  // namespace tlct::_cvt
