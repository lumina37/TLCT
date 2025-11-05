#include <expected>
#include <new>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/patch_merge/cache.hpp"
#endif

namespace tlct::_cvt::pm {

template <cfg::concepts::CArrange TArrange>
MvCache_<TArrange>::MvCache_(cv::Mat&& gradBlendingWeight, cv::Mat&& renderCanvas, cv::Mat&& weightCanvas) noexcept
    : gradBlendingWeight(std::move(gradBlendingWeight)),
      renderCanvas(std::move(renderCanvas)),
      weightCanvas(std::move(weightCanvas)) {}

template <cfg::concepts::CArrange TArrange>
auto MvCache_<TArrange>::create(const TMvParams& params) noexcept -> std::expected<MvCache_, Error> {
    try {
        constexpr float GRADIENT_BLENDING_WIDTH = 0.75;
        cv::Mat gradBlendingWeight = circleWithFadeoutBorder(params.resizedPatchWidth, GRADIENT_BLENDING_WIDTH);
        cv::Mat renderCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        cv::Mat weightCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        return MvCache_{std::move(gradBlendingWeight), std::move(renderCanvas), std::move(weightCanvas)};
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ECate::eSys, ECode::eOutOfMemory}};
    }
}

template class MvCache_<cfg::CornersArrange>;
template class MvCache_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::pm
