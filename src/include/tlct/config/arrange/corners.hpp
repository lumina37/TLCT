#pragma once

#include <array>
#include <numbers>

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
    TLCT_API inline CornersArrange() noexcept
        : imgsize_(),
          diameter_(),
          radius_(),
          direction_(),
          left_top_(),
          right_top_(),
          left_y_unit_shift_(),
          right_y_unit_shift_(),
          mirows_(),
          micols_(),
          upsample_(1),
          is_out_shift_(){};
    TLCT_API inline CornersArrange(const CornersArrange& rhs) noexcept = default;
    TLCT_API inline CornersArrange& operator=(const CornersArrange& rhs) noexcept = default;
    TLCT_API inline CornersArrange(CornersArrange&& rhs) noexcept = default;
    TLCT_API inline CornersArrange& operator=(CornersArrange&& rhs) noexcept = default;
    TLCT_API inline CornersArrange(cv::Size imgsize, float diameter, bool direction, cv::Point2f left_top,
                                   cv::Point2f right_top, cv::Point2f left_bottom, cv::Point2f right_bottom) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline CornersArrange fromCfgMap(const ConfigMap& map);

    // Non-const methods
    TLCT_API inline CornersArrange& upsample(int factor) noexcept;

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
    cv::Point2f right_top_;
    cv::Point2f left_y_unit_shift_;
    cv::Point2f right_y_unit_shift_;
    int mirows_;
    TMiCols micols_;
    int upsample_;
    bool is_out_shift_;
};

CornersArrange CornersArrange::fromCfgMap(const ConfigMap& map) {
    cv::Size imgsize{map.get<"LensletWidth", int>(), map.get<"LensletHeight", int>()};
    const float diameter = map.get<"MIDiameter", float>();
    const bool direction = map.get_or<"MLADirection">(false);
    const cv::Point2f left_top = {map.get<"LeftTopMICenterX", float>(), map.get<"LeftTopMICenterY", float>()};
    const cv::Point2f right_top = {map.get<"RightTopMICenterX", float>(), map.get<"RightTopMICenterY", float>()};
    const cv::Point2f left_bottom = {map.get<"LeftBottomMICenterX", float>(), map.get<"LeftBottomMICenterY", float>()};
    const cv::Point2f right_bottom = {map.get<"RightBottomMICenterX", float>(),
                                      map.get<"RightBottomMICenterY", float>()};

    return {imgsize, diameter, direction, left_top, right_top, left_bottom, right_bottom};
}

CornersArrange& CornersArrange::upsample(int factor) noexcept {
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    left_top_ *= factor;
    right_top_ *= factor;
    left_y_unit_shift_ *= factor;
    right_y_unit_shift_ *= factor;
    upsample_ = factor;
    return *this;
}

cv::Point2f CornersArrange::getMICenter(int row, int col) const noexcept {
    cv::Point2f left = left_top_ + left_y_unit_shift_ * row;
    cv::Point2f right = right_top_ + right_y_unit_shift_ * row;
    cv::Point2f x_unit_shift = (right - left) / (micols_[0] - 1);
    cv::Point2f center = left + x_unit_shift * col;

    if (row % 2 == 1) {
        center -= x_unit_shift / 2.f * _hp::sgn(isOutShift());
    }

    return center;
}

cv::Point2f CornersArrange::getMICenter(cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

CornersArrange::CornersArrange(cv::Size imgsize, float diameter, bool direction, cv::Point2f left_top,
                               cv::Point2f right_top, cv::Point2f left_bottom, cv::Point2f right_bottom) noexcept
    : diameter_(diameter), radius_(diameter / 2.f), direction_(direction), upsample_(1) {
    if (direction) {
        std::swap(left_top.x, left_top.y);
        std::swap(right_bottom.x, right_bottom.y);
        std::swap(right_top.x, right_top.y);
        std::swap(left_bottom.x, left_bottom.y);
        std::swap(right_top, left_bottom);
        std::swap(imgsize.height, imgsize.width);
    }
    imgsize_ = imgsize;
    left_top_ = left_top;
    right_top_ = right_top;

    const auto veclen = [](const cv::Point2f vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y); };

    const cv::Point2f top_x_shift = right_top - left_top;
    const int top_cols = _hp::iround(veclen(top_x_shift) / diameter) + 1;
    const cv::Point2f top_x_unit_shift = top_x_shift / (top_cols - 1);

    if (left_top.x < top_x_unit_shift.x) {
        is_out_shift_ = false;
    } else {
        is_out_shift_ = true;
    }

    micols_ = {top_cols, top_cols};
    if (is_out_shift_) {
        // Now the second row have one more intact MI than the first row
        const float mi_1_0_x = left_top.x - top_x_unit_shift.x / 2.f;
        if (mi_1_0_x + top_x_unit_shift.x * top_cols < imgsize.width) {
            micols_[1]++;
        }
    } else {
        // Now the second row have one less intact MI than the first row
        const float mi_1_0_x = left_top.x + top_x_unit_shift.x / 2.f;
        if (mi_1_0_x + top_x_unit_shift.x * top_cols >= imgsize.width) {
            micols_[1]--;
        }
    }

    const cv::Point2f left_y_shift = left_bottom - left_top;
    const float approx_y_unit_shift = diameter * std::numbers::sqrt3_v<float> / 2.f;
    const int left_y_rows = _hp::iround(veclen(left_y_shift) / approx_y_unit_shift) + 1;
    left_y_unit_shift_ = left_y_shift / (left_y_rows - 1);
    mirows_ = (int)(((float)imgsize.height - diameter / 2.f - left_top.y) / left_y_unit_shift_.y) + 1;

    const cv::Point2f right_y_shift = right_bottom - right_top;
    right_y_unit_shift_ = right_y_shift / (left_y_rows - 1);
}

}  // namespace tlct::_cfg
