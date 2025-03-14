#include <cmath>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/common/map.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/helper/constexpr/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/arrange/offset.hpp"
#endif

namespace tlct::_cfg {

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

static_assert(concepts::CArrange<OffsetArrange>);

}  // namespace tlct::_cfg
