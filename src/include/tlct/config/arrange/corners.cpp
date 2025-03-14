#include <cmath>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/common/map.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/helper/constexpr/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/arrange/corners.hpp"
#endif

namespace tlct::_cfg {

CornersArrange::CornersArrange(cv::Size imgSize, float diameter, bool direction, cv::Point2f leftTop,
                               cv::Point2f rightTop, cv::Point2f leftBottom, cv::Point2f rightBottom) noexcept
    : diameter_(diameter), radius_(diameter / 2.f), direction_(direction), upsample_(1) {
    if (direction) {
        std::swap(leftTop.x, leftTop.y);
        std::swap(rightBottom.x, rightBottom.y);
        std::swap(rightTop.x, rightTop.y);
        std::swap(leftBottom.x, leftBottom.y);
        std::swap(rightTop, leftBottom);
        std::swap(imgSize.height, imgSize.width);
    }
    imgSize_ = imgSize;
    leftTop_ = leftTop;
    rightTop_ = rightTop;

    const auto veclen = [](const cv::Point2f vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y); };

    const cv::Point2f topXShift = rightTop - leftTop;
    const int topCols = _hp::iround(veclen(topXShift) / diameter) + 1;
    const cv::Point2f topXUnitShift = topXShift / (topCols - 1);

    if (leftTop.x < topXUnitShift.x) {
        isOutShift_ = false;
    } else {
        isOutShift_ = true;
    }

    miCols_ = {topCols, topCols};
    if (isOutShift_) {
        // Now the second row have one more intact MI than the first row
        const float mi10X = leftTop.x - topXUnitShift.x / 2.f;
        if (mi10X + topXUnitShift.x * topCols < imgSize.width) {
            miCols_[1]++;
        }
    } else {
        // Now the second row have one less intact MI than the first row
        const float mi10X = leftTop.x + topXUnitShift.x / 2.f;
        if (mi10X + topXUnitShift.x * topCols >= imgSize.width) {
            miCols_[1]--;
        }
    }

    const cv::Point2f leftYShift = leftBottom - leftTop;
    const float approxYUnitShift = diameter * std::numbers::sqrt3_v<float> / 2.f;
    const int leftYRows = _hp::iround(veclen(leftYShift) / approxYUnitShift) + 1;
    leftYUnitShift_ = leftYShift / (leftYRows - 1);
    miRows_ = (int)(((float)imgSize.height - diameter / 2.f - leftTop.y) / leftYUnitShift_.y) + 1;

    const cv::Point2f rightYShift = rightBottom - rightTop;
    rightYUnitShift_ = rightYShift / (leftYRows - 1);
}

CornersArrange CornersArrange::fromCfgMap(const ConfigMap& map) {
    cv::Size imgSize{map.get<"LensletWidth", int>(), map.get<"LensletHeight", int>()};
    const float diameter = map.get<"MIDiameter", float>();
    const bool direction = map.getOr<"MLADirection">(false);
    const cv::Point2f leftTop = {map.get<"LeftTopMICenterX", float>(), map.get<"LeftTopMICenterY", float>()};
    const cv::Point2f rightTop = {map.get<"RightTopMICenterX", float>(), map.get<"RightTopMICenterY", float>()};
    const cv::Point2f leftBottom = {map.get<"LeftBottomMICenterX", float>(), map.get<"LeftBottomMICenterY", float>()};
    const cv::Point2f rightBottom = {map.get<"RightBottomMICenterX", float>(),
                                     map.get<"RightBottomMICenterY", float>()};

    return {imgSize, diameter, direction, leftTop, rightTop, leftBottom, rightBottom};
}

CornersArrange& CornersArrange::upsample(int factor) noexcept {
    imgSize_ *= factor;
    diameter_ *= (float)factor;
    radius_ *= (float)factor;
    leftTop_ *= factor;
    rightTop_ *= factor;
    leftYUnitShift_ *= factor;
    rightYUnitShift_ *= factor;
    upsample_ = factor;
    return *this;
}

cv::Point2f CornersArrange::getMICenter(int row, int col) const noexcept {
    cv::Point2f left = leftTop_ + leftYUnitShift_ * row;
    cv::Point2f right = rightTop_ + rightYUnitShift_ * row;
    cv::Point2f xUnitShift = (right - left) / (miCols_[0] - 1);
    cv::Point2f center = left + xUnitShift * col;

    if (row % 2 == 1) {
        center -= xUnitShift / 2.f * _hp::sgn(isOutShift());
    }

    return center;
}

cv::Point2f CornersArrange::getMICenter(cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

static_assert(concepts::CArrange<CornersArrange>);

}  // namespace tlct::_cfg
