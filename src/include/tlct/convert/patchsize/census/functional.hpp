#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt::census {

TLCT_API void censusTransform5x5(const cv::Mat& src, const cv::Mat& srcMask, cv::Mat& censusMap,
                                 cv::Mat& censusMask) noexcept;

}  // namespace tlct::_cvt::census

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/census/functional.cpp"
#endif
