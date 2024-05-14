#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct {

template <typename T>
inline void transposeCenters_(const cv::Mat& src, cv::Mat& dst)
{
    using PT = cv::Point_<T>;
    cv::transpose(src, dst);
    for (int row = 0; row < dst.rows; row++) {
        auto prow = dst.ptr<PT>(row);
        for (int col = 0; col < dst.cols; col++) {
            PT& c = prow[col];
            std::swap(c.x, c.y);
        }
    }
}

template <typename T>
inline cv::Mat transposeCenters(const cv::Mat& src)
{
    cv::Mat dst;
    transposeCenters_<T>(src, dst);
    return dst;
}

} // namespace tlct