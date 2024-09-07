#pragma once

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

static inline cv::Mat expand_mat(const cv::Mat& src)
{
    cv::Mat result;
    src.convertTo(result, CV_32F);
    return std::move(result);
}

class QualitySSIM
{
public:
    inline cv::Scalar compute(const cv::Mat& cmp);

    static inline cv::Ptr<QualitySSIM> create(const cv::Mat& ref);

private:
    // holds computed values for a mat
    struct _mat_data {
        cv::Mat I, I_2, mu, mu_2, sigma_2;

        // construct from mat_type
        explicit _mat_data(const cv::Mat&);

        // computes ssim and quality map for single frame
        static cv::Scalar compute(const _mat_data& lhs, const _mat_data& rhs);

    }; // mat_data

    _mat_data _refImgData;

    explicit QualitySSIM(_mat_data refImgData) : _refImgData(std::move(refImgData)) {}
};

static inline cv::Mat blur(const cv::Mat& mat)
{
    cv::Mat result;
    cv::GaussianBlur(mat, result, cv::Size(11, 11), 1.5);
    return result;
}

QualitySSIM::_mat_data::_mat_data(const cv::Mat& mat)
{
    this->I = expand_mat(mat);
    cv::multiply(this->I, this->I, this->I_2);
    this->mu = blur(this->I);
    cv::multiply(this->mu, this->mu, this->mu_2);
    this->sigma_2 = blur(this->I_2); // blur the squared img, subtract blurred_squared
    cv::subtract(this->sigma_2, this->mu_2, this->sigma_2);
}

cv::Ptr<QualitySSIM> QualitySSIM::create(const cv::Mat& ref) { return {new QualitySSIM(_mat_data(ref))}; }

cv::Scalar QualitySSIM::compute(const cv::Mat& cmp)
{
    auto result = _mat_data::compute(this->_refImgData, _mat_data(cmp));
    return result;
}

cv::Scalar QualitySSIM::_mat_data::compute(const _mat_data& lhs, const _mat_data& rhs)
{
    constexpr double C1 = 6.5025, C2 = 58.5225;

    cv::Mat I1_I2, mu1_mu2, t1, t2, t3, sigma12;

    cv::multiply(lhs.I, rhs.I, I1_I2);
    cv::multiply(lhs.mu, rhs.mu, mu1_mu2);
    cv::subtract(blur(I1_I2), mu1_mu2, sigma12);

    // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    cv::multiply(mu1_mu2, 2., t1);
    cv::add(t1, C1, t1); // t1 += C1

    cv::multiply(sigma12, 2., t2);
    cv::add(t2, C2, t2); // t2 += C2

    // t3 = t1 * t2
    cv::multiply(t1, t2, t3);

    // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
    cv::add(lhs.mu_2, rhs.mu_2, t1);
    cv::add(t1, C1, t1);

    cv::add(lhs.sigma_2, rhs.sigma_2, t2);
    cv::add(t2, C2, t2);

    // t1 *= t2
    cv::multiply(t1, t2, t1);

    // quality map: t3 /= t1
    cv::divide(t3, t1, t3);

    return cv::mean(t3);
}

} // namespace tlct::_cvt
