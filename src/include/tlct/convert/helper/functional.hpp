#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

namespace rgs = std::ranges;

[[nodiscard]] TLCT_API cv::Mat circleWithFadeoutBorder(int diameter, float borderWidthFactor) noexcept;

struct Grads {
    float deg0;
    float deg30;
    float deg60;
    float deg90;
    float normed;
};

[[nodiscard]] TLCT_API Grads computeGrads(const cv::Mat& src) noexcept;

[[nodiscard]] TLCT_API uint16_t dhash(const cv::Mat& src);

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/functional.cpp"
#endif
