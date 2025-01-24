#pragma once

#include <opencv2/imgproc.hpp>

#include "tlct/convert/helper.hpp"

namespace tlct::_cvt {

class WrapSSIM {
public:
    // Constructor
    WrapSSIM() = delete;
    inline explicit WrapSSIM(const MIBuffer& mi) noexcept : mi_(mi) {};
    inline WrapSSIM(const WrapSSIM& rhs) = default;
    WrapSSIM& operator=(const WrapSSIM& rhs) = delete;
    WrapSSIM(WrapSSIM&& rhs) noexcept = default;
    WrapSSIM& operator=(WrapSSIM&& rhs) noexcept = delete;

    // Const methods
    [[nodiscard]] inline float compare(const WrapSSIM& rhs) const noexcept;

    // Non-const methods
    inline void updateRoi(cv::Rect roi) noexcept;

    const MIBuffer& mi_;
    cv::Mat I_, I2_, mu_, mu2_, sigma2_;

private:
    mutable cv::Mat I1I2, mu1mu2, sigma12;
};

void WrapSSIM::updateRoi(cv::Rect roi) noexcept {
    mi_.I(roi).copyTo(I_);  // `BORDER_ISOLATED` has no effect, so we must copy here
    mi_.I_2(roi).copyTo(I2_);
    blurInto(I_, mu_);
    cv::multiply(mu_, mu_, mu2_);
    blurInto(I2_, sigma2_);
    cv::subtract(sigma2_, mu2_, sigma2_);
}

float WrapSSIM::compare(const WrapSSIM& rhs) const noexcept {
    constexpr float C1 = 6.5025f, C2 = 58.5225f;

    cv::multiply(I_, rhs.I_, I1I2);
    cv::multiply(mu_, rhs.mu_, mu1mu2);
    blurInto(I1I2, I1I2);
    cv::subtract(I1I2, mu1mu2, sigma12);

    // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cv::Mat& t1 = I1I2;
    cv::multiply(mu1mu2, 2., t1);
    cv::add(t1, C1, t1);  // t1 += C1

    cv::Mat& t2 = mu1mu2;
    cv::multiply(sigma12, 2., t2);
    cv::add(t2, C2, t2);  // t2 += C2

    // t3 = t1 * t2
    cv::Mat& t3 = sigma12;
    cv::multiply(t1, t2, t3);

    // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    cv::add(mu2_, rhs.mu2_, t1);
    cv::add(t1, C1, t1);

    cv::add(sigma2_, rhs.sigma2_, t2);
    cv::add(t2, C2, t2);

    // t1 *= t2
    cv::multiply(t1, t2, t1);

    // quality map: t3 /= t1
    cv::divide(t3, t1, t3);

    const cv::Scalar& ssimScalar = cv::mean(t3);
    const float ssim = (float)ssimScalar[0];

    return ssim;
}

}  // namespace tlct::_cvt
