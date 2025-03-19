#pragma once

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class MvParams_ {
public:
    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = tcfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] static MvParams_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg);

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

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/params.cpp"
#endif
