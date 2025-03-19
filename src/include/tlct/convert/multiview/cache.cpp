#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/multiview/params.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/cache.hpp"
#endif

namespace tlct::_cvt {

template <tcfg::concepts::CArrange TArrange>
MvCache_<TArrange> MvCache_<TArrange>::fromParams(const MvParams_<TArrange>& params) {
    constexpr float GRADIENT_BLENDING_WIDTH = 0.75;
    cv::Mat gradBlendingWeight = circleWithFadeoutBorder(params.resizedPatchWidth, GRADIENT_BLENDING_WIDTH);
    cv::Mat renderCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
    cv::Mat weightCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
    return {std::move(gradBlendingWeight), std::move(renderCanvas), std::move(weightCanvas)};
}

template class MvCache_<_cfg::CornersArrange>;
template class MvCache_<_cfg::OffsetArrange>;

}  // namespace tlct::_cvt
