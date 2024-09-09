#pragma once

#include <opencv2/imgproc.hpp>

#include "functional.hpp"
#include "microimages.hpp"
#include "tlct/common/defines.h"

namespace tlct::_cvt {

class WrapSSIM
{
public:
    // Constructor
    TLCT_API inline explicit WrapSSIM(const WrapMI& mi) noexcept : mi_(mi){};
    TLCT_API inline WrapSSIM(const WrapSSIM& rhs) = default;
    WrapSSIM& operator=(const WrapSSIM& rhs) = delete;
    TLCT_API WrapSSIM(WrapSSIM&& rhs) noexcept = default;
    WrapSSIM& operator=(WrapSSIM&& rhs) noexcept = delete;

    // Const methods
    [[nodiscard]] TLCT_API inline double compare(const WrapSSIM& rhs) const noexcept;

    // Non-const methods
    inline void updateRoi(cv::Rect roi) noexcept;

    const WrapMI& mi_;
    cv::Mat I_, I_2_, mu_, mu_2_, sigma_2_;
};

void WrapSSIM::updateRoi(cv::Rect roi) noexcept
{
    I_ = mi_.I_(roi);
    I_2_ = mi_.I_2_(roi);
    blur(I_, mu_);
    cv::multiply(mu_, mu_, mu_2_);
    blur(I_2_, sigma_2_);
    cv::subtract(sigma_2_, mu_2_, sigma_2_);
}

double WrapSSIM::compare(const WrapSSIM& rhs) const noexcept
{
    constexpr double C1 = 6.5025, C2 = 58.5225;

    cv::Mat I1_I2, mu1_mu2, t1, t2, t3, sigma12;

    cv::multiply(I_, rhs.I_, I1_I2);
    cv::multiply(mu_, rhs.mu_, mu1_mu2);
    blur(I1_I2, I1_I2);
    cv::subtract(I1_I2, mu1_mu2, sigma12);

    // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cv::multiply(mu1_mu2, 2., t1);
    cv::add(t1, C1, t1); // t1 += C1

    cv::multiply(sigma12, 2., t2);
    cv::add(t2, C2, t2); // t2 += C2

    // t3 = t1 * t2
    cv::multiply(t1, t2, t3);

    // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    cv::add(mu_2_, rhs.mu_2_, t1);
    cv::add(t1, C1, t1);

    cv::add(sigma_2_, rhs.sigma_2_, t2);
    cv::add(t2, C2, t2);

    // t1 *= t2
    cv::multiply(t1, t2, t1);

    // quality map: t3 /= t1
    cv::divide(t3, t1, t3);

    const auto ssim = cv::mean(t3);

    return -ssim[0];
}

} // namespace tlct::_cvt
