#pragma once

#include <numbers>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CLayout TLayout_>
class MvParams_
{
public:
    // Typename alias
    using TLayout = TLayout_;
    using TCvtConfig = tcfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] static inline MvParams_ fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg);

    cv::Range canvas_crop_roi[2];
    double psize_inflate;
    int views;
    int patch_xshift; // the extracted patch will be zoomed to this height
    int patch_yshift;
    int resized_patch_width;
    int view_interval;
    int canvas_width;
    int canvas_height;
    int output_width;
    int output_height;
};

template <tcfg::concepts::CLayout TLayout>
MvParams_<TLayout> MvParams_<TLayout>::fromConfigs(const TLayout& layout, const TCvtConfig& cvt_cfg)
{
    const double psize_inflate = cvt_cfg.psize_inflate;

    const double patch_xshift_d = 0.3 * layout.getDiameter();
    const int patch_xshift = (int)std::ceil(patch_xshift_d);
    const int patch_yshift = (int)std::ceil(patch_xshift_d * std::numbers::sqrt3 / 2.0);

    const double p_resize_d = patch_xshift_d * cvt_cfg.psize_inflate;
    const int resized_patch_width = (int)std::round(p_resize_d);

    const int view_shift_range = _hp::iround(layout.getDiameter() * cvt_cfg.view_shift_range);
    const int view_interval = cvt_cfg.views > 1 ? view_shift_range / (cvt_cfg.views - 1) : 0;

    const int canvas_width = (int)std::round(layout.getMIMaxCols() * patch_xshift + resized_patch_width);
    const int canvas_height = (int)std::round(layout.getMIRows() * patch_yshift + resized_patch_width);

    const cv::Range col_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_width - resized_patch_width - patch_xshift / 2.0)};
    const cv::Range row_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_height - resized_patch_width - patch_xshift / 2.0)};

    const int upsample = layout.getUpsample();
    const int output_width = _hp::roundTo<2>(_hp::iround((double)col_range.size() / upsample));
    const int output_height = _hp::roundTo<2>(_hp::iround((double)row_range.size() / upsample));

    return {{row_range, col_range}, psize_inflate, cvt_cfg.views, patch_xshift, patch_yshift, resized_patch_width,
            view_interval,          canvas_width,  canvas_height, output_width, output_height};
}

} // namespace tlct::_cvt
