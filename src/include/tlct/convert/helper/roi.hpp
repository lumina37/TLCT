#pragma once

#include <cmath>

#include <opencv2/core.hpp>

namespace tlct::cvt::_hp {

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2d& center, const double width) noexcept
{
    const int startx = (int)(center.x - width / 2.0);
    const int starty = (int)(center.y - width / 2.0);
    const int width_i = std::ceil(width);
    cv::Mat roi = src({startx, starty, width_i, width_i});
    return std::move(roi);
}

static inline cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, const int width) noexcept
{
    const int startx = center.x - width / 2;
    const int starty = center.y - width / 2;
    cv::Mat roi = src({startx, starty, width, width});
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

} // namespace tlct::cvt::_hp