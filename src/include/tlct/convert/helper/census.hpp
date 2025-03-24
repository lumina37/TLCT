#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

TLCT_API void censusTransform5x5(const cv::Mat& src, const cv::Mat& srcMask, cv::Mat& censusMap, cv::Mat& censusMask);

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/census.cpp"
#endif
