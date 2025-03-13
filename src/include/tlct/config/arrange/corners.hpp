#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common/map.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cfg {

class CornersArrange {
public:
    // Typename alias
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API CornersArrange() noexcept
        : imgSize_(),
          diameter_(),
          radius_(),
          direction_(),
          leftTop_(),
          rightTop_(),
          leftYUnitShift_(),
          rightYUnitShift_(),
          miRows_(),
          miCols_(),
          upsample_(1),
          isOutShift_() {};
    TLCT_API CornersArrange(const CornersArrange& rhs) noexcept = default;
    TLCT_API CornersArrange& operator=(const CornersArrange& rhs) noexcept = default;
    TLCT_API CornersArrange(CornersArrange&& rhs) noexcept = default;
    TLCT_API CornersArrange& operator=(CornersArrange&& rhs) noexcept = default;
    TLCT_API CornersArrange(cv::Size imgSize, float diameter, bool direction, cv::Point2f leftTop, cv::Point2f rightTop,
                            cv::Point2f leftBottom, cv::Point2f rightBottom) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static CornersArrange fromCfgMap(const ConfigMap& map);

    // Non-const methods
    TLCT_API CornersArrange& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API int getImgWidth() const noexcept { return imgSize_.width; };
    [[nodiscard]] TLCT_API int getImgHeight() const noexcept { return imgSize_.height; };
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept { return imgSize_; };
    [[nodiscard]] TLCT_API float getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API float getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API bool getDirection() const noexcept { return direction_; };
    [[nodiscard]] TLCT_API int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API int getMIRows() const noexcept { return miRows_; };
    [[nodiscard]] TLCT_API int getMICols(const int row) const noexcept { return miCols_[row % miCols_.size()]; };
    [[nodiscard]] TLCT_API int getMIMaxCols() const noexcept { return std::max(miCols_[0], miCols_[1]); };
    [[nodiscard]] TLCT_API int getMIMinCols() const noexcept { return std::min(miCols_[0], miCols_[1]); };
    [[nodiscard]] TLCT_API cv::Point2f getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API cv::Point2f getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API bool isOutShift() const noexcept { return isOutShift_; };

private:
    cv::Size imgSize_;
    float diameter_;
    float radius_;
    bool direction_;
    cv::Point2f leftTop_;
    cv::Point2f rightTop_;
    cv::Point2f leftYUnitShift_;
    cv::Point2f rightYUnitShift_;
    int miRows_;
    TMiCols miCols_;
    int upsample_;
    bool isOutShift_;
};

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

}  // namespace tlct::_cfg
