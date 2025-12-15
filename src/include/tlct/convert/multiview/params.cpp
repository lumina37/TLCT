#include <cmath>
#include <numbers>

#include <opencv2/core.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/params.hpp"
#endif

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange>
auto MvParams_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<MvParams_, Error> {
    const float psizeInflate = cvtCfg.psizeInflate;

    const float safeDiameter = arrange.getDiameter() * CONTENT_SAFE_RATIO;
    const float maxPsize = safeDiameter * (1.f - cvtCfg.viewShiftRange);

    const float patchXShift = 0.37f * arrange.getDiameter();
    const float patchYShift = patchXShift * std::numbers::sqrt3_v<float> / 2.f;

    const float f32ResizedPatchWdt = patchXShift * cvtCfg.psizeInflate;
    const int resizedPatchWdt = _hp::iround(f32ResizedPatchWdt);

    const float viewShiftRange = safeDiameter * cvtCfg.viewShiftRange;
    const float viewInterval = cvtCfg.views > 1 ? viewShiftRange / (float)(cvtCfg.views - 1) : 0;

    const int canvasWidth = _hp::iround(arrange.getMIMaxCols() * patchXShift + resizedPatchWdt);
    const int canvasHeight = _hp::iround(arrange.getMIRows() * patchYShift + resizedPatchWdt);

    const cv::Range colRange{(int)std::ceil(patchXShift * 1.5),
                             (int)(canvasWidth - resizedPatchWdt - patchXShift / 2.f)};
    const cv::Range rowRange{(int)std::ceil(patchXShift * 1.5),
                             (int)(canvasHeight - resizedPatchWdt - patchXShift / 2.f)};

    const int upsample = arrange.getUpsample();
    const int outputWidth = _hp::roundTo<2>(_hp::iround((float)colRange.size() / upsample));
    const int outputHeight = _hp::roundTo<2>(_hp::iround((float)rowRange.size() / upsample));

    return MvParams_{{rowRange, colRange}, psizeInflate, cvtCfg.views, maxPsize,     patchXShift, patchYShift,
                     resizedPatchWdt,      viewInterval, canvasWidth,  canvasHeight, outputWidth, outputHeight};
}

template class MvParams_<cfg::CornersArrange>;
template class MvParams_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
