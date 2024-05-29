#pragma once

#include <opencv2/imgproc.hpp>

namespace tlct::cvt::tspc::_hp {

static inline cv::Mat rectWithFadeoutBorder(const cv::Size size, const int border_width)
{
    cv::Mat rect = cv::Mat::ones(size, CV_64FC1);

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

} // namespace tlct::cvt::tspc::_hp
