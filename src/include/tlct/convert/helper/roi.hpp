#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

[[nodiscard]] TLCT_API cv::Rect getRoiByCenter(const cv::Point2f& center, float width) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center, float width) noexcept;

[[nodiscard]] TLCT_API cv::Rect getRoiByCenter(const cv::Point2f& center, cv::Size2f size) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point2f& center,
                                                   cv::Size2f size) noexcept;

[[nodiscard]] TLCT_API cv::Rect getRoiByCenter(const cv::Point& center, int width) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, int width) noexcept;

[[nodiscard]] TLCT_API cv::Rect getRoiByCenter(const cv::Point& center, cv::Size size) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByCenter(const cv::Mat& src, const cv::Point& center, cv::Size size) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                         float width) noexcept;

[[nodiscard]] TLCT_API cv::Mat getRoiImageByLeftupCorner(const cv::Mat& src, const cv::Point& corner,
                                                         int width) noexcept;

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/roi.cpp"
#endif
