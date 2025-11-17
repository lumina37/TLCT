#pragma once

#include <opencv2/core.hpp>

#include "tlct/helper/std.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] TLCT_API cv::Mat circleWithFadeoutBorder(int diameter, float borderWidthFactor) noexcept;

[[nodiscard]] TLCT_API float computeGrads(const cv::Mat& src) noexcept;

TLCT_API void computeGradsMap(const cv::Mat& src, cv::Mat& dst) noexcept;

[[nodiscard]] TLCT_API uint16_t computeDhash(const cv::Mat& src);

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.cpp"
#endif
