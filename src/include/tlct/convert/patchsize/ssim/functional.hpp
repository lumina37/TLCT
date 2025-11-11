#pragma once

#include <opencv2/core.hpp>

#include "tlct/convert/patchsize/ssim/mibuffer.hpp"

namespace tlct::_cvt::ssim {

class WrapSSIM {
public:
    // Constructor
    WrapSSIM() = delete;
    WrapSSIM(const WrapSSIM& rhs) = default;
    WrapSSIM& operator=(const WrapSSIM& rhs) = delete;
    WrapSSIM(WrapSSIM&& rhs) noexcept = default;
    WrapSSIM& operator=(WrapSSIM&& rhs) noexcept = delete;

    explicit WrapSSIM(const MIBuffer& mi) noexcept : mi_(mi) {};

    // Const methods
    [[nodiscard]] float compare(const WrapSSIM& rhs) const noexcept;

    // Non-const methods
    void updateRoi(cv::Rect roi) noexcept;

    const MIBuffer& mi_;
    cv::Mat I_, I2_, mu_, mu2_, sigma2_;

private:
    mutable cv::Mat I1I2, mu1mu2, sigma12;
};

void blurInto(const cv::Mat& src, cv::Mat& dst);

}  // namespace tlct::_cvt::ssim

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/functional.cpp"
#endif
