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

class OffsetArrange {
public:
    // Typename alias
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API inline OffsetArrange() noexcept
        : imgSize_(),
          diameter_(),
          radius_(),
          direction_(),
          leftTop_(),
          xUnitShift_(),
          yUnitShift_(),
          miRows_(),
          miCols_(),
          upsample_(1),
          isOutShift_(){};
    TLCT_API inline OffsetArrange(const OffsetArrange& rhs) noexcept = default;
    TLCT_API inline OffsetArrange& operator=(const OffsetArrange& rhs) noexcept = default;
    TLCT_API inline OffsetArrange(OffsetArrange&& rhs) noexcept = default;
    TLCT_API inline OffsetArrange& operator=(OffsetArrange&& rhs) noexcept = default;
    TLCT_API inline OffsetArrange(cv::Size imgSize, float diameter, bool direction, cv::Point2f offset) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline OffsetArrange fromCfgMap(const ConfigMap& map);

    // Non-const methods
    TLCT_API inline OffsetArrange& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgSize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgSize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgSize_; };
    [[nodiscard]] TLCT_API inline float getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline float getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline bool getDirection() const noexcept { return direction_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return miRows_; };
    [[nodiscard]] TLCT_API inline int getMICols(const int row) const noexcept { return miCols_[row % miCols_.size()]; };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(miCols_[0], miCols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(miCols_[0], miCols_[1]); };
    [[nodiscard]] TLCT_API inline cv::Point2f getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2f getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return isOutShift_; };

private:
    cv::Size imgSize_;
    float diameter_;
    float radius_;
    bool direction_;
    cv::Point2f leftTop_;
    float xUnitShift_;
    float yUnitShift_;
    int miRows_;
    TMiCols miCols_;
    int upsample_;
    bool isOutShift_;
};

OffsetArrange OffsetArrange::fromCfgMap(const ConfigMap& map) {
    cv::Size imgSize{map.get<"LensletWidth", int>(), map.get<"LensletHeight", int>()};
    const float diameter = map.get<"MIDiameter", float>();
    const bool direction = map.getOr<"MLADirection">(false);
    const cv::Point2f offset = {map.get<"CentralMIOffsetX", float>(), map.get<"CentralMIOffsetY", float>()};

    return {imgSize, diameter, direction, offset};
}

OffsetArrange& OffsetArrange::upsample(int factor) noexcept {
    imgSize_ *= factor;
    diameter_ *= (float)factor;
    radius_ *= (float)factor;
    leftTop_ *= factor;
    xUnitShift_ *= (float)factor;
    yUnitShift_ *= (float)factor;
    upsample_ = factor;
    return *this;
}

cv::Point2f OffsetArrange::getMICenter(int row, int col) const noexcept {
    cv::Point2f center{leftTop_.x + xUnitShift_ * (float)col, leftTop_.y + yUnitShift_ * (float)row};
    if (row % 2 == 1) {
        center.x -= xUnitShift_ / 2.f * (float)_hp::sgn(isOutShift());
    }
    return center;
}

cv::Point2f OffsetArrange::getMICenter(cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

OffsetArrange::OffsetArrange(cv::Size imgSize, float diameter, bool direction, cv::Point2f offset) noexcept
    : diameter_(diameter), radius_(diameter / 2.f), direction_(direction), upsample_(1) {
    cv::Point2f centerMI{imgSize.width / 2.f + offset.x, imgSize.height / 2.f - offset.y};

    if (getDirection()) {
        std::swap(imgSize.height, imgSize.width);
        std::swap(centerMI.x, centerMI.y);
    }
    imgSize_ = imgSize;

    xUnitShift_ = diameter;
    yUnitShift_ = diameter * std::numbers::sqrt3_v<float> / 2.f;
    const int centerMIXIdx = (int)((centerMI.x - radius_) / xUnitShift_);
    const int centerMIYIdx = (int)((centerMI.y - radius_) / yUnitShift_);

    const float leftX = centerMI.x - xUnitShift_ * centerMIXIdx;
    if (centerMIYIdx % 2 == 0) {
        leftTop_.x = leftX;
        if (leftTop_.x > diameter) {
            isOutShift_ = true;
        } else {
            isOutShift_ = false;
        }
    } else {
        if (leftX > diameter) {
            leftTop_.x = leftX - radius_;
            isOutShift_ = false;
        } else {
            leftTop_.x = leftX + radius_;
            isOutShift_ = true;
        }
    }
    leftTop_.y = centerMI.y - std::floor((centerMI.y - yUnitShift_ / 2.f) / yUnitShift_) * yUnitShift_;

    const float mi_1_0_x = leftTop_.x - xUnitShift_ / 2.f * (float)_hp::sgn(isOutShift_);
    miCols_[0] = (int)(((float)imgSize.width - leftTop_.x - xUnitShift_ / 2.f) / xUnitShift_) + 1;
    miCols_[1] = (int)(((float)imgSize.width - mi_1_0_x - xUnitShift_ / 2.f) / xUnitShift_) + 1;
    miRows_ = (int)(((float)imgSize.height - leftTop_.y - yUnitShift_ / 2.f) / yUnitShift_) + 1;
}

}  // namespace tlct::_cfg
