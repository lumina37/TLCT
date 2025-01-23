#pragma once

#include <array>
#include <numbers>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common/map.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cfg {

class OffsetArrange
{
public:
    // Typename alias
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API inline OffsetArrange() noexcept
        : imgsize_(), diameter_(), radius_(), direction_(), left_top_(), x_unit_shift_(), y_unit_shift_(), mirows_(),
          micols_(), upsample_(1), is_out_shift_(){};
    TLCT_API inline OffsetArrange(const OffsetArrange& rhs) noexcept = default;
    TLCT_API inline OffsetArrange& operator=(const OffsetArrange& rhs) noexcept = default;
    TLCT_API inline OffsetArrange(OffsetArrange&& rhs) noexcept = default;
    TLCT_API inline OffsetArrange& operator=(OffsetArrange&& rhs) noexcept = default;
    TLCT_API inline OffsetArrange(cv::Size imgsize, float diameter, bool direction, cv::Point2f offset) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline OffsetArrange fromCfgMap(const ConfigMap& map);

    // Non-const methods
    TLCT_API inline OffsetArrange& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline float getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline float getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline bool getDirection() const noexcept { return direction_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return mirows_; };
    [[nodiscard]] TLCT_API inline int getMICols(const int row) const noexcept { return micols_[row % micols_.size()]; };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline cv::Point2f getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2f getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return is_out_shift_; };

private:
    cv::Size imgsize_;
    float diameter_;
    float radius_;
    bool direction_;
    cv::Point2f left_top_;
    float x_unit_shift_;
    float y_unit_shift_;
    int mirows_;
    TMiCols micols_;
    int upsample_;
    bool is_out_shift_;
};

OffsetArrange OffsetArrange::fromCfgMap(const ConfigMap& map)
{
    cv::Size imgsize{map.get<"LensletWidth", int>(), map.get<"LensletHeight", int>()};
    const float diameter = map.get<"MIDiameter", float>();
    const bool direction = map.get_or<"MLADirection">(false);
    const cv::Point2f offset = {map.get<"CentralMIOffsetX", float>(), map.get<"CentralMIOffsetY", float>()};

    return {imgsize, diameter, direction, offset};
}

OffsetArrange& OffsetArrange::upsample(int factor) noexcept
{

    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    left_top_ *= factor;
    x_unit_shift_ *= factor;
    y_unit_shift_ *= factor;
    upsample_ = factor;
    return *this;
}

cv::Point2f OffsetArrange::getMICenter(int row, int col) const noexcept
{
    cv::Point2f center{left_top_.x + x_unit_shift_ * col, left_top_.y + y_unit_shift_ * row};
    if (row % 2 == 1) {
        center.x -= x_unit_shift_ / 2.0 * _hp::sgn(isOutShift());
    }
    return center;
}

cv::Point2f OffsetArrange::getMICenter(cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

OffsetArrange::OffsetArrange(cv::Size imgsize, float diameter, bool direction, cv::Point2f offset) noexcept
    : diameter_(diameter), radius_(diameter / 2.0f), direction_(direction), upsample_(1)
{
    cv::Point2f center_mi{imgsize.width / 2.0f + offset.x, imgsize.height / 2.0f - offset.y};

    if (getDirection()) {
        std::swap(imgsize.height, imgsize.width);
        std::swap(center_mi.x, center_mi.y);
    }
    imgsize_ = imgsize;

    x_unit_shift_ = diameter;
    y_unit_shift_ = diameter * std::numbers::sqrt3 / 2.0;
    const int center_mi_xidx = (int)((center_mi.x - radius_) / x_unit_shift_);
    const int center_mi_yidx = (int)((center_mi.y - radius_) / y_unit_shift_);

    const float left_x = center_mi.x - x_unit_shift_ * center_mi_xidx;
    if (center_mi_yidx % 2 == 0) {
        left_top_.x = left_x;
        if (left_top_.x > diameter) {
            is_out_shift_ = true;
        } else {
            is_out_shift_ = false;
        }
    } else {
        if (left_x > diameter) {
            left_top_.x = left_x - radius_;
            is_out_shift_ = false;
        } else {
            left_top_.x = left_x + radius_;
            is_out_shift_ = true;
        }
    }
    left_top_.y = center_mi.y - std::floor((center_mi.y - y_unit_shift_ / 2.0) / y_unit_shift_) * y_unit_shift_;

    const float mi_1_0_x = left_top_.x - x_unit_shift_ / 2.0 * _hp::sgn(is_out_shift_);
    micols_[0] = (int)(((float)imgsize.width - left_top_.x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    micols_[1] = (int)(((float)imgsize.width - mi_1_0_x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    mirows_ = (int)(((float)imgsize.height - left_top_.y - y_unit_shift_ / 2.0) / y_unit_shift_) + 1;
}

} // namespace tlct::_cfg
