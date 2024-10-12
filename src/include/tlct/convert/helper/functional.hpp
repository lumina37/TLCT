#pragma once

#include <array>
#include <bit>
#include <cmath>
#include <numeric>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] static inline cv::Mat circleWithFadeoutBorder(const int diameter, const double border_width_factor)
{
    cv::Mat rect = cv::Mat::zeros({diameter, diameter}, CV_32FC1);
    const double radius = (double)diameter / 2.0;
    const double heap = border_width_factor > 0.0 ? 1.0 + 1.0 / border_width_factor * (1.0 - border_width_factor)
                                                  : std::numeric_limits<double>::max();

    for (const int row : rgs::views::iota(0, diameter)) {
        auto prow = rect.ptr<float>(row);
        for (const int col : rgs::views::iota(0, diameter)) {
            const double xdist = radius - (double)row;
            const double ydist = radius - (double)col;
            const double dist = std::sqrt(xdist * xdist + ydist * ydist);
            const double pix = std::max(0.0, std::min(1.0, (1.0 - dist / radius) * heap));
            *prow = (float)pix;
            prow++;
        }
    }

    return rect;
}

[[nodiscard]] static inline double grad(const cv::Mat& src)
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

[[nodiscard]] static inline uint64_t dhash(const cv::Mat& src)
{
    constexpr int thumbnail_height = 8;
    constexpr int thumbnail_width = thumbnail_height + 1;
    constexpr int thumbnail_size = thumbnail_height * thumbnail_width;
    std::array<float, thumbnail_size> thumbnail_buffer;
    cv::Mat thumbnail(thumbnail_height, thumbnail_width, CV_32FC1, thumbnail_buffer.data());
    cv::resize(src, thumbnail, {thumbnail_width, thumbnail_height});

    uint64_t dhash = 0;
    const uint64_t u64max = std::numeric_limits<uint64_t>::max();
    uint64_t mask = u64max ^ (u64max >> 1);
    for (const int row : rgs::views::iota(0, thumbnail_height)) {
        auto prow = thumbnail.ptr<float>(row);
        for ([[maybe_unused]] const int _ : rgs::views::iota(0, thumbnail_width)) {
            const bool flag = *(prow + 1) > *prow;
            dhash |= flag * mask;
            mask >>= 1;
            prow++;
        }
    }

    return dhash;
}

[[nodiscard]] static inline int L1_dist(uint64_t lhs, uint64_t rhs) { return std::popcount(lhs ^ rhs); }

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
