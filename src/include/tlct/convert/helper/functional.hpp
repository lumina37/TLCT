#pragma once

#include <numeric>
#include <vector>

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

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

static inline void blur_(const cv::Mat& src, cv::Mat& dst) { cv::GaussianBlur(src, dst, {11, 11}, 1.5); }

} // namespace tlct::_cvt
