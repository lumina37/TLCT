#include <new>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/ltype_merge/cache.hpp"
#endif

namespace tlct::_cvt::lm {

template <cfg::concepts::CArrange TArrange>
MvCache_<TArrange>::MvCache_(cv::Mat&& renderCanvas, cv::Mat&& weightCanvas, cv::Mat&& gradsCanvas) noexcept
    : renderCanvas(std::move(renderCanvas)),
      weightCanvas(std::move(weightCanvas)),
      gradsWeightCanvas(std::move(gradsCanvas)) {}

template <cfg::concepts::CArrange TArrange>
auto MvCache_<TArrange>::create(const TMvParams& params) noexcept -> std::expected<MvCache_, Error> {
    try {
        cv::Mat renderCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        cv::Mat weightCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        cv::Mat gradsCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
        return MvCache_{std::move(renderCanvas), std::move(weightCanvas), std::move(gradsCanvas)};
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ECate::eSys, ECode::eOutOfMemory}};
    }
}

template class MvCache_<cfg::CornersArrange>;
template class MvCache_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::lm
