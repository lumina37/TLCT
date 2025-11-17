#pragma once

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange_>
class MvParams_ {
public:
    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = cfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MvParams_, Error> create(const TArrange& arrange,
                                                                         const TCvtConfig& cvtCfg) noexcept;

    [[nodiscard]] cv::Size getRoiSize() const noexcept { return {canvasCropRoi[1].size(), canvasCropRoi[0].size()}; }

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
