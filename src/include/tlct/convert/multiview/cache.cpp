#include <expected>
#include <new>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/helper/error.hpp"
#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/multiview/params.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/cache.hpp"
#endif

namespace tlct::_cvt {

template <tcfg::concepts::CArrange TArrange>
std::expected<MvCache_<TArrange>, Error> MvCache_<TArrange>::create(const MvParams_<TArrange>& params) noexcept {
    constexpr float GRADIENT_BLENDING_WIDTH = 0.75;

    try {
        cv::Mat gradBlendingWeight = circleWithFadeoutBorder(params.resizedPatchWidth, GRADIENT_BLENDING_WIDTH);
        cv::Mat renderCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        cv::Mat weightCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        return MvCache_{std::move(gradBlendingWeight), std::move(renderCanvas), std::move(weightCanvas)};
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ErrCode::OutOfMemory}};
    }
}

template class MvCache_<_cfg::CornersArrange>;
template class MvCache_<_cfg::OffsetArrange>;

}  // namespace tlct::_cvt
