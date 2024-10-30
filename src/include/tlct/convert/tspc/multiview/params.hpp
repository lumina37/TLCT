#pragma once

#include "tlct/config/tspc.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt::tspc {

namespace tcfg = tlct::cfg::tspc;

class MvParams
{
public:
    // Typename alias
    using TLayout = tcfg::Layout;
    using TSpecificConfig = TLayout::TSpecificConfig;

    // Initialize from
    [[nodiscard]] static inline MvParams fromConfigs(const TLayout& layout, const TSpecificConfig& spec_cfg, int views);

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

MvParams MvParams::fromConfigs(const TLayout& layout, const TSpecificConfig& spec_cfg, int views)
{
    const double psize_inflate = spec_cfg.getPsizeInflate();

    const double patch_xshift_d = 0.35 * layout.getDiameter();
    const int patch_xshift = (int)std::ceil(patch_xshift_d);
    const int patch_yshift = (int)std::ceil(patch_xshift_d * std::numbers::sqrt3 / 2.0);

    const double p_resize_d = patch_xshift_d * spec_cfg.getPsizeInflate();
    const int resized_patch_width = (int)std::round(p_resize_d);

    const int move_range =
        _hp::iround(layout.getDiameter() * (1.0 - spec_cfg.getMaxPsize() * spec_cfg.getPsizeInflate()));
    const int view_interval = views > 1 ? move_range / (views - 1) : 0;

    const int canvas_width = (int)std::round(layout.getMIMaxCols() * patch_xshift + resized_patch_width);
    const int canvas_height = (int)std::round(layout.getMIRows() * patch_yshift + resized_patch_width);

    const cv::Range col_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_width - resized_patch_width - patch_xshift / 2.0)};
    const cv::Range row_range{(int)std::ceil(patch_xshift * 1.5),
                              (int)(canvas_height - resized_patch_width - patch_xshift / 2.0)};

    const int upsample = layout.getUpsample();
    const int output_width = _hp::roundTo<2>(_hp::iround((double)col_range.size() / upsample));
    const int output_height = _hp::roundTo<2>(_hp::iround((double)row_range.size() / upsample));

    return {{row_range, col_range}, psize_inflate, views,         patch_xshift, patch_yshift, resized_patch_width,
            view_interval,          canvas_width,  canvas_height, output_width, output_height};
}

} // namespace tlct::_cvt::tspc
