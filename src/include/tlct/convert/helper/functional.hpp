#pragma once

#include <numeric>
#include <vector>

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

[[nodiscard]] static inline cv::Mat circleWithFadeoutBorder(const int diameter, const int border_width)
{
    cv::Mat rect = cv::Mat::zeros({diameter, diameter}, CV_32FC1);
    constexpr int bitshift = 4; // supports 1/16 pixel
    constexpr int compensate = 1 << (bitshift - 1);
    const int shifted_border_width = border_width << bitshift;
    const int radius = ((diameter << bitshift) + compensate) / 2;
    const cv::Point center{radius, radius};

    constexpr double max_color = 1.0;
    cv::circle(rect, center, radius - shifted_border_width, max_color, cv::FILLED, cv::LINE_8, bitshift);

    const double color_step = max_color / (shifted_border_width + 1);
    double color = color_step;
    for (int i = 0; i < shifted_border_width; i++) {
        cv::circle(rect, center, radius - i, color, 1, cv::LINE_8, bitshift);
        color += color_step;
    }

    return rect;
}

[[nodiscard]] static inline double computeGrad(const cv::Mat& src)
{
    cv::Mat edges;
    double weight = 0.0;
    cv::Sobel(src, edges, CV_16S, 1, 0);
    edges = cv::abs(edges);
    weight += cv::sum(edges)[0];
    cv::Sobel(src, edges, CV_16S, 0, 1);
    edges = cv::abs(edges);
    weight += cv::sum(edges)[0];
    weight /= edges.size().area();
    return weight;
}

template <typename Tv>
[[nodiscard]] inline Tv stdvar(const std::vector<Tv>& vec)
{
    const Tv sum = std::reduce(vec.begin(), vec.end());
    const Tv avg = sum / (Tv)vec.size();

    Tv var = 0.0;
    for (const Tv elem : vec) {
        const Tv diff = elem - avg;
        var += diff * diff;
    }
    Tv stdvar = var / (Tv)vec.size();

    return stdvar;
}

static inline void blurInto(const cv::Mat& src, cv::Mat& dst) { cv::GaussianBlur(src, dst, {11, 11}, 1.5); }

} // namespace tlct::_cvt
