#pragma once

#include <opencv2/core.hpp>

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] cv::Mat circleWithFadeoutBorder(const int diameter, const float borderWidthFactor);

[[nodiscard]] float textureIntensity(const cv::Mat& src);

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.cpp"
#endif
