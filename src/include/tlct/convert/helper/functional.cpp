#include <cmath>
#include <cstdint>
#include <limits>
#include <ranges>

#include <opencv2/imgproc.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] cv::Mat circleWithFadeoutBorder(const int diameter, const float borderWidthFactor) {
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

[[nodiscard]] float textureIntensity(const cv::Mat& src) {
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

}  // namespace tlct::_cvt
