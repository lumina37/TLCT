#pragma once

#include <opencv2/imgproc.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cvt {

static inline cv::Mat blur(const cv::Mat& mat)
{
    cv::Mat result;
    cv::GaussianBlur(mat, result, cv::Size(11, 11), 1.5);
    return result;
}

class WrapSSIM
{
public:
    // Constructor
    TLCT_API explicit inline WrapSSIM(const cv::Mat& src);
    TLCT_API WrapSSIM& operator=(const WrapSSIM& rhs) noexcept = default;
    TLCT_API inline WrapSSIM(const WrapSSIM& rhs) noexcept = default;
    TLCT_API WrapSSIM& operator=(WrapSSIM&& rhs) noexcept = default;
    TLCT_API WrapSSIM(WrapSSIM&& rhs) noexcept = default;

    // Const methods
    [[nodiscard]] TLCT_API inline double compare(const WrapSSIM& rhs) const;

    // Init from
    [[nodiscard]] static inline WrapSSIM fromRoi(const cv::Mat& roi);

private:
    cv::Mat I, I_2, mu, mu_2, sigma_2;
};

WrapSSIM WrapSSIM::fromRoi(const cv::Mat& roi) { return WrapSSIM(roi); }

double WrapSSIM::compare(const WrapSSIM& rhs) const
{
    constexpr double C1 = 6.5025, C2 = 58.5225;

    cv::Mat I1_I2, mu1_mu2, t1, t2, t3, sigma12;

    cv::multiply(I, rhs.I, I1_I2);
    cv::multiply(mu, rhs.mu, mu1_mu2);
    cv::subtract(blur(I1_I2), mu1_mu2, sigma12);

    // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cv::multiply(mu1_mu2, 2., t1);
    cv::add(t1, C1, t1); // t1 += C1

    cv::multiply(sigma12, 2., t2);
    cv::add(t2, C2, t2); // t2 += C2

    // t3 = t1 * t2
    cv::multiply(t1, t2, t3);

    // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    cv::add(mu_2, rhs.mu_2, t1);
    cv::add(t1, C1, t1);

    cv::add(sigma_2, rhs.sigma_2, t2);
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
    src.convertTo(this->I, CV_32F);
    cv::multiply(this->I, this->I, this->I_2);
    this->mu = blur(this->I);
    cv::multiply(this->mu, this->mu, this->mu_2);
    this->sigma_2 = blur(this->I_2); // blur the squared img, subtract blurred_squared
    cv::subtract(this->sigma_2, this->mu_2, this->sigma_2);
}

} // namespace tlct::_cvt
