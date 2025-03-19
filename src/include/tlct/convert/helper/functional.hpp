#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] TLCT_API cv::Mat circleWithFadeoutBorder(int diameter, float borderWidthFactor);

[[nodiscard]] TLCT_API float textureIntensity(const cv::Mat& src);

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.cpp"
#endif
