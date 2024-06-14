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

static inline cv::Mat rectWithFadeoutBorder(const cv::Size size, const int border_width)
{
    cv::Mat rect = cv::Mat::ones(size, CV_32FC1);

    cv::Point lefttop{0, 0};
    cv::Point rightbot{rect.cols - 1, rect.rows - 1};
    const cv::Point vertex_step{1, 1};

    constexpr double max_color = 1.0;
    const double color_step = max_color / (border_width + 1);
    double color = color_step;
    for (int i = 0; i < border_width; i++) {
        cv::rectangle(rect, lefttop, rightbot, color, 1);
        color += color_step;
        lefttop += vertex_step;
        rightbot -= vertex_step;
    }
    return std::move(rect);
}

} // namespace tlct::cvt::_hp