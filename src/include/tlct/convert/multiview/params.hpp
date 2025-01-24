#pragma once

#include <numbers>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
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
    [[nodiscard]] static inline MvParams_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvt_cfg);

    cv::Range canvas_crop_roi[2];
    float psize_inflate;
    int views;
    int patch_xshift;  // the extracted patch will be zoomed to this height
    int patch_yshift;
    int resized_patch_width;
    int view_interval;
    int canvas_width;
    int canvas_height;
    int output_width;
    int output_height;
};

template <tcfg::concepts::CArrange TArrange>
MvParams_<TArrange> MvParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvt_cfg) {
    const float psize_inflate = cvt_cfg.psize_inflate;

    const float patch_xshift_f = 0.3f * arrange.getDiameter();
    const int patch_xshift = (int)std::ceil(patch_xshift_f);
    const int patch_yshift = (int)std::ceil(patch_xshift_f * std::numbers::sqrt3_v<float> / 2.f);

    const float resized_patch_wdt_f = patch_xshift_f * cvt_cfg.psize_inflate;
    const int resized_patch_wdt = (int)std::roundf(resized_patch_wdt_f);

    const int view_shift_range = _hp::iround(arrange.getDiameter() * cvt_cfg.view_shift_range);
    const int view_interval = cvt_cfg.views > 1 ? view_shift_range / (cvt_cfg.views - 1) : 0;

    const int canvas_width = (int)std::roundf(arrange.getMIMaxCols() * patch_xshift_f + resized_patch_wdt_f);
    const int canvas_height = (int)std::roundf(arrange.getMIRows() * patch_xshift_f + resized_patch_wdt_f);

    const cv::Range col_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_width - resized_patch_wdt - patch_xshift / 2.f)};
    const cv::Range row_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_height - resized_patch_wdt - patch_xshift / 2.f)};

    const int upsample = arrange.getUpsample();
    const int output_width = _hp::roundTo<2>(_hp::iround((float)col_range.size() / upsample));
    const int output_height = _hp::roundTo<2>(_hp::iround((float)row_range.size() / upsample));

    return {{row_range, col_range}, psize_inflate, cvt_cfg.views, patch_xshift, patch_yshift, resized_patch_wdt,
            view_interval,          canvas_width,  canvas_height, output_width, output_height};
}

}  // namespace tlct::_cvt
