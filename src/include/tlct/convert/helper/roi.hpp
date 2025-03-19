#pragma once

#include <opencv2/core.hpp>

namespace tlct::_cvt {

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point2f& center, const float width) noexcept;

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center, const float width) noexcept;

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point2f& center, const cv::Size2f size) noexcept;

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center,
                                          const cv::Size2f size) noexcept;

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point& center, const int width) noexcept;

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, const int width) noexcept;

[[nodiscard]] cv::Rect getRoiByCenter(const cv::Point& center, const cv::Size size) noexcept;

[[nodiscard]] cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, const cv::Size size) noexcept;

[[nodiscard]] cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                const float width) noexcept;

[[nodiscard]] cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner, const int width) noexcept;

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/roi.cpp"
#endif
