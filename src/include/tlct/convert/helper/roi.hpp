#pragma once

#include <cmath>

#include <opencv2/core.hpp>

namespace tlct::_cvt {

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2d& center, const double width) noexcept
{
    const int startx = (int)std::round(center.x - width / 2.0);
    const int starty = (int)std::round(center.y - width / 2.0);
    const int width_i = (int)std::round(width);
    cv::Mat roi = src({startx, starty, width_i, width_i});
    return std::move(roi);
}

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2d& center, const cv::Size2d size) noexcept
{
    const int startx = (int)std::round(center.x - size.width / 2.0);
    const int starty = (int)std::round(center.y - size.height / 2.0);
    const int width_i = (int)std::round(size.width);
    const int height_i = (int)std::round(size.width);
    cv::Mat roi = src({startx, starty, width_i, height_i});
    return std::move(roi);
}

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, const int width) noexcept
{
    const int startx = center.x - (width + 1) / 2;
    const int starty = center.y - (width + 1) / 2;
    cv::Mat roi = src({startx, starty, width, width});
    return std::move(roi);
}

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, const cv::Size size) noexcept
{
    const int startx = center.x - (size.width + 1) / 2;
    const int starty = center.y - (size.height + 1) / 2;
    cv::Mat roi = src({startx, starty, size.width, size.height});
    return std::move(roi);
}

static inline cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                const double width) noexcept
{
    const int width_i = (int)std::ceil(width);
    const cv::Rect rect(corner.x, corner.y, width_i, width_i);
    cv::Mat roi = src(rect);
    return std::move(roi);
}

static inline cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner, const int width) noexcept
{
    const cv::Rect rect(corner.x, corner.y, width, width);
    cv::Mat roi = src(rect);
    return std::move(roi);
}

static inline cv::Mat circleWithFadeoutBorder(const int diameter, const int border_width)
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
    return std::move(rect);
}

} // namespace tlct::_cvt
