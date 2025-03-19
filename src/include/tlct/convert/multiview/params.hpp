#pragma once

#include <cmath>
#include <numbers>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class MvParams_ {
public:
    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = tcfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] static inline MvParams_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg);

    cv::Range canvasCropRoi[2];
    float psizeInflate;
    int views;
    int patchXShift;  // the extracted patch will be zoomed to this height
    int patchYShift;
    int resizedPatchWidth;
    int viewInterval;
    int canvasWidth;
    int canvasHeight;
    int outputWidth;
    int outputHeight;
};

template <tcfg::concepts::CArrange TArrange>
MvParams_<TArrange> MvParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg) {
    const float psizeInflate = cvtCfg.psizeInflate;

    const float f32PatchXShift = 0.3f * arrange.getDiameter();
    const int patchXShift = (int)std::ceil(f32PatchXShift);
    const int patchYShift = (int)std::ceil(f32PatchXShift * std::numbers::sqrt3_v<float> / 2.f);

    const float f32ResizedPatchWdt = f32PatchXShift * cvtCfg.psizeInflate;
    const int resizedPatchWdt = (int)std::roundf(f32ResizedPatchWdt);

    const int viewShiftRange = _hp::iround(arrange.getDiameter() * SAFE_RATIO * cvtCfg.viewShiftRange);
    const int viewInterval = cvtCfg.views > 1 ? viewShiftRange / (cvtCfg.views - 1) : 0;

    const int canvasWidth = arrange.getMIMaxCols() * patchXShift + resizedPatchWdt;
    const int canvasHeight = arrange.getMIRows() * patchYShift + resizedPatchWdt;

    const cv::Range colRange{(int)std::ceil(patchXShift * 1.5),
                             (int)(canvasWidth - resizedPatchWdt - f32PatchXShift / 2.f)};
    const cv::Range rowRange{(int)std::ceil(patchXShift * 1.5),
                             (int)(canvasHeight - resizedPatchWdt - f32PatchXShift / 2.f)};

    const int upsample = arrange.getUpsample();
    const int outputWidth = _hp::roundTo<2>(_hp::iround((float)colRange.size() / upsample));
    const int outputHeight = _hp::roundTo<2>(_hp::iround((float)rowRange.size() / upsample));

    return {{rowRange, colRange}, psizeInflate, cvtCfg.views, patchXShift, patchYShift, resizedPatchWdt,
            viewInterval,         canvasWidth,  canvasHeight, outputWidth, outputHeight};
}

}  // namespace tlct::_cvt
