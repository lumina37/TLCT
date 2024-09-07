#pragma once

#include <opencv2/imgproc.hpp>

namespace tlct::_cvt {

static constexpr const int EXPANDED_MAT_DEFAULT_TYPE = CV_32F;

template <typename R>
inline R extract_mat(cv::InputArray in, const int type = -1)
{
    R result = {};
    if (in.isMat())
        in.getMat().convertTo(result, (type != -1) ? type : in.getMat().type());
    else if (in.isUMat())
        in.getUMat().convertTo(result, (type != -1) ? type : in.getUMat().type());

    return result;
}

template <typename R>
inline R expand_mat(cv::InputArray src, int TYPE_DEFAULT = EXPANDED_MAT_DEFAULT_TYPE)
{
    auto result = extract_mat<R>(src, -1);

    // by default, expand to 32F unless we already have >= 32 bits, then go to 64
    //  if/when we can detect OpenCL CV_16F support, opt for that when input depth == 8
    //  note that this may impact the precision of the algorithms and would need testing
    int type = TYPE_DEFAULT;

    switch (result.depth()) {
    case CV_32F:
    case CV_32S:
    case CV_64F:
        type = CV_64F;
    }; // switch

    result.convertTo(result, type);
    return result;
}

using _mat_type = cv::UMat;

class QualitySSIM
{
public:
    inline cv::Scalar compute(cv::InputArray cmp);

    static inline cv::Ptr<QualitySSIM> create(cv::InputArray ref);

protected:
    // holds computed values for a mat
    struct _mat_data {
        _mat_type I, I_2, mu, mu_2, sigma_2;

        // allow default construction
        _mat_data() = default;

        // construct from mat_type
        _mat_data(const _mat_type&);

        // construct from inputarray
        _mat_data(cv::InputArray);

        // return flag if this is empty
        bool empty() const { return I.empty() && I_2.empty() && mu.empty() && mu_2.empty() && sigma_2.empty(); }

        // computes ssim and quality map for single frame
        static std::pair<cv::Scalar, _mat_type> compute(const _mat_data& lhs, const _mat_data& rhs);

    }; // mat_data

    _mat_type _qualityMap;
    _mat_data _refImgData;

    explicit QualitySSIM(_mat_data refImgData) : _refImgData(std::move(refImgData)) {}
};

// SSIM blur function
static inline _mat_type blur(const _mat_type& mat)
{
    _mat_type result = {};
    cv::GaussianBlur(mat, result, cv::Size(11, 11), 1.5);
    return result;
}

QualitySSIM::_mat_data::_mat_data(const _mat_type& mat)
{
    this->I = mat;
    cv::multiply(this->I, this->I, this->I_2);
    this->mu = blur(this->I);
    cv::multiply(this->mu, this->mu, this->mu_2);
    this->sigma_2 = blur(this->I_2); // blur the squared img, subtract blurred_squared
    cv::subtract(this->sigma_2, this->mu_2, this->sigma_2);
}

QualitySSIM::_mat_data::_mat_data(cv::InputArray arr) : _mat_data(expand_mat<_mat_type>(arr)) // delegate
{
}

// static
cv::Ptr<QualitySSIM> QualitySSIM::create(cv::InputArray ref)
{
    return cv::Ptr<QualitySSIM>(new QualitySSIM(_mat_data(ref)));
}

cv::Scalar QualitySSIM::compute(cv::InputArray cmp)
{
    auto result = _mat_data::compute(this->_refImgData, _mat_data(cmp));

    cv::OutputArray(this->_qualityMap).assign(result.second);
    return result.first;
}

std::pair<cv::Scalar, _mat_type> QualitySSIM::_mat_data::compute(const _mat_data& lhs, const _mat_data& rhs)
{
    const double C1 = 6.5025, C2 = 58.5225;

    _mat_type I1_I2, mu1_mu2, t1, t2, t3, sigma12;

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

    return {cv::mean(t3), std::move(t3)};
} // compute

} // namespace tlct::_cvt
