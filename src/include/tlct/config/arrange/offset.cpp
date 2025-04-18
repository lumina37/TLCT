#include <cmath>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/common/error.hpp"
#include "tlct/config/common/map.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/helper/constexpr/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/arrange/offset.hpp"
#endif

namespace tlct::_cfg {

std::expected<OffsetArrange, Error> OffsetArrange::create(cv::Size imgSize, float diameter, bool direction,
                                                          cv::Point2f offset) noexcept {
    cv::Point2f centerMI{imgSize.width / 2.f + offset.x, imgSize.height / 2.f - offset.y};

    if (direction) {
        std::swap(imgSize.height, imgSize.width);
        std::swap(centerMI.x, centerMI.y);
    }

    const float radius = diameter / 2.f;
    const float xUnitShift = diameter;
    const float yUnitShift = diameter * std::numbers::sqrt3_v<float> / 2.f;
    const int centerMIXIdx = (int)((centerMI.x - radius) / xUnitShift);
    const int centerMIYIdx = (int)((centerMI.y - radius) / yUnitShift);

    cv::Point2f leftTop;
    bool isOutShift;
    const float leftX = centerMI.x - xUnitShift * centerMIXIdx;
    if (centerMIYIdx % 2 == 0) {
        leftTop.x = leftX;
        if (leftTop.x > diameter) {
            isOutShift = true;
        } else {
            isOutShift = false;
        }
    } else {
        if (leftX > diameter) {
            leftTop.x = leftX - radius;
            isOutShift = false;
        } else {
            leftTop.x = leftX + radius;
            isOutShift = true;
        }
    }
    leftTop.y = centerMI.y - std::floor((centerMI.y - yUnitShift / 2.f) / yUnitShift) * yUnitShift;

    TMiCols miCols;
    const float mi_1_0_x = leftTop.x - xUnitShift / 2.f * (float)_hp::sgn(isOutShift);
    miCols[0] = (int)(((float)imgSize.width - leftTop.x - xUnitShift / 2.f) / xUnitShift) + 1;
    miCols[1] = (int)(((float)imgSize.width - mi_1_0_x - xUnitShift / 2.f) / xUnitShift) + 1;
    const int miRows = (int)(((float)imgSize.height - leftTop.y - yUnitShift / 2.f) / yUnitShift) + 1;

    return OffsetArrange{imgSize, diameter, leftTop, xUnitShift, yUnitShift, miRows, miCols, 1, direction, isOutShift};
}

std::expected<OffsetArrange, Error> OffsetArrange::createWithCfgMap(const ConfigMap& map) noexcept {
    cv::Size imgSize{map.get<"LensletWidth", int>(), map.get<"LensletHeight", int>()};
    const float diameter = map.get<"MIDiameter", float>();
    const bool direction = map.getOr<"MLADirection">(false);
    const cv::Point2f offset = {map.get<"CentralMIOffsetX", float>(), map.get<"CentralMIOffsetY", float>()};

    return create(imgSize, diameter, direction, offset);
}

OffsetArrange& OffsetArrange::upsample(int factor) noexcept {
    imgSize_ *= factor;
    diameter_ *= (float)factor;
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
