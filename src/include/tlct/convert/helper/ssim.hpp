#pragma once

#include <opencv2/imgproc.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

static inline void blur(const cv::Mat& src, cv::Mat& dst) { cv::GaussianBlur(src, dst, {11, 11}, 1.5); }

class WrapSSIM
{
public:
    // Constructor
    TLCT_API explicit inline WrapSSIM(const cv::Mat& src);
    TLCT_API WrapSSIM& operator=(const WrapSSIM& rhs) = default;
    TLCT_API inline WrapSSIM(const WrapSSIM& rhs) = default;
    TLCT_API WrapSSIM& operator=(WrapSSIM&& rhs) noexcept = default;
    TLCT_API WrapSSIM(WrapSSIM&& rhs) noexcept = default;

    // Const methods
    [[nodiscard]] TLCT_API inline double compare(const WrapSSIM& rhs) const;

    // Init from
    [[nodiscard]] static inline WrapSSIM fromRoi(const cv::Mat& roi);

private:
    cv::Mat I_, I_2_, mu_, mu_2_, sigma_2_;
};

WrapSSIM WrapSSIM::fromRoi(const cv::Mat& roi) { return WrapSSIM(roi); }

double WrapSSIM::compare(const WrapSSIM& rhs) const
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

WrapSSIM::WrapSSIM(const cv::Mat& src)
{
    src.convertTo(I_, CV_32F);
    cv::multiply(I_, I_, I_2_);
    blur(I_, mu_);
    cv::multiply(mu_, mu_, mu_2_);
    blur(I_2_, sigma_2_);
    cv::subtract(sigma_2_, mu_2_, sigma_2_);
}

} // namespace tlct::_cvt
