#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/quality.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cvt::tspc::_hp {

namespace tcfg = tlct::cfg::tspc;

class AnchorWrapper
{
public:
    // Typename alias
    using TBase = cv::quality::QualitySSIM;

    // Constructor
    TLCT_API AnchorWrapper& operator=(const AnchorWrapper& rhs) noexcept = default;
    TLCT_API inline AnchorWrapper(const AnchorWrapper& rhs) noexcept = default;
    TLCT_API AnchorWrapper& operator=(AnchorWrapper&& rhs) noexcept = default;
    TLCT_API AnchorWrapper(AnchorWrapper&& rhs) noexcept = default;
    TLCT_API inline AnchorWrapper(cv::Ptr<cv::quality::QualitySSIM>&& base, const double weight)
        : base_(std::move(base)), weight_(weight){};

    // Const methods
    [[nodiscard]] TLCT_API inline double getWeight() const noexcept;
    [[nodiscard]] TLCT_API inline double compare(const cv::Mat& rhs) const;

    // Init from
    [[nodiscard]] static inline AnchorWrapper fromRoi(const cv::Mat& roi);

private:
    cv::Ptr<TBase> base_;
    double weight_;
};

AnchorWrapper AnchorWrapper::fromRoi(const cv::Mat& roi)
{
    cv::Mat edges;
    double weight = 0.0;
    cv::Sobel(roi, edges, CV_16S, 1, 0);
    edges = cv::abs(edges);
    weight += cv::sum(edges)[0];
    cv::Sobel(roi, edges, CV_16S, 0, 1);
    edges = cv::abs(edges);
    weight += cv::sum(edges)[0];
    weight /= edges.size().area();
    auto base = TBase::create(roi);
    return {std::move(base), weight};
}

double AnchorWrapper::getWeight() const noexcept { return weight_; }

double AnchorWrapper::compare(const cv::Mat& rhs) const { return -base_->compute(rhs)[0]; }

} // namespace tlct::cvt::tspc::_hp
