#pragma once

#include <cmath>

#include <opencv2/core.hpp>

namespace tlct::_cvt {

[[nodiscard]] static inline cv::Rect getRoiByCenter(const cv::Point2d& center, const double width) noexcept
{
    const int startx = (int)std::round(center.x - width / 2.0);
    const int starty = (int)std::round(center.y - width / 2.0);
    const int width_i = (int)std::round(width);
    return {startx, starty, width_i, width_i};
}

[[nodiscard]] static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2d& center,
                                                        const double width) noexcept
{
    cv::Mat roi = src(getRoiByCenter(center, width));
    return roi;
}

[[nodiscard]] static inline cv::Rect getRoiByCenter(const cv::Point2d& center, const cv::Size2d size) noexcept
{
    const int startx = (int)std::round(center.x - size.width / 2.0);
    const int starty = (int)std::round(center.y - size.height / 2.0);
    const int width_i = (int)std::round(size.width);
    const int height_i = (int)std::round(size.width);
    return {startx, starty, width_i, height_i};
}

[[nodiscard]] static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2d& center,
                                                        const cv::Size2d size) noexcept
{
    cv::Mat roi = src(getRoiByCenter(center, size));
    return roi;
}

[[nodiscard]] static inline cv::Rect getRoiByCenter(const cv::Point& center, const int width) noexcept
{
    const int startx = center.x - (width + 1) / 2;
    const int starty = center.y - (width + 1) / 2;
    return {startx, starty, width, width};
}

[[nodiscard]] static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center,
                                                        const int width) noexcept
{
    cv::Mat roi = src(getRoiByCenter(center, width));
    return roi;
}

[[nodiscard]] static inline cv::Rect getRoiByCenter(const cv::Point& center, const cv::Size size) noexcept
{
    const int startx = center.x - (size.width + 1) / 2;
    const int starty = center.y - (size.height + 1) / 2;
    return {startx, starty, size.width, size.height};
}

[[nodiscard]] static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center,
                                                        const cv::Size size) noexcept
{
    cv::Mat roi = src(getRoiByCenter(center, size));
    return roi;
}

[[nodiscard]] static inline cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                              const double width) noexcept
{
    const int width_i = (int)std::round(width);
    cv::Mat roi = src({corner.x, corner.y, width_i, width_i});
    return roi;
}

[[nodiscard]] static inline cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                              const int width) noexcept
{
    cv::Mat roi = src({corner.x, corner.y, width, width});
    return roi;
}

} // namespace tlct::_cvt
