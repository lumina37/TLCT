#pragma once

#include <numbers>

#include <opencv2/imgproc.hpp>

#include "param.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::_cfg::tspc {

class Layout
{
public:
    // Typename alias
    using TParamConfig = ParamConfig;
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API inline Layout() noexcept
        : left_top_(), right_top_(), is_out_shift_(), left_y_unit_shift_(), right_y_unit_shift_(), mirows_(), micols_(),
          imgsize_(), diameter_(), radius_(), rotation_(), upsample_(1){};
    TLCT_API inline Layout(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(cv::Point2d left_top, cv::Point2d right_top, bool is_out_shift,
                           cv::Point2d left_y_unit_shift, cv::Point2d right_y_unit_shift, int mirows, TMiCols micols,
                           cv::Size imgsize, double diameter, double rotation) noexcept
        : left_top_(left_top), right_top_(right_top), is_out_shift_(is_out_shift),
          left_y_unit_shift_(left_y_unit_shift), right_y_unit_shift_(right_y_unit_shift), mirows_(mirows),
          micols_(micols), imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0), rotation_(rotation),
          upsample_(1){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline Layout fromParamConfig(const TParamConfig& cfg);

    // Non-const methods
    TLCT_API inline Layout& upsample(const int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline double getRotation() const noexcept { return rotation_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(const int row, const int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(const cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return mirows_; };
    [[nodiscard]] TLCT_API inline int getMICols(const int row) const noexcept { return micols_[row % micols_.size()]; };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return is_out_shift_; };
    [[nodiscard]] TLCT_API inline int isOutShiftSgn() const noexcept { return _hp::sgn(isOutShift()); };

    TLCT_API inline void procImg_(const cv::Mat& src, cv::Mat& dst) const;
    [[nodiscard]] TLCT_API inline cv::Mat procImg(const cv::Mat& src) const;

private:
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_y_unit_shift_;
    cv::Point2d right_y_unit_shift_;
    double diameter_;
    double radius_;
    double rotation_;
    cv::Size imgsize_;
    TMiCols micols_;
    int mirows_;
    int upsample_;
    bool is_out_shift_;
};

static_assert(concepts::CLayout<Layout>);

Layout Layout::fromParamConfig(const TParamConfig& cfg)
{
    const auto& calib_cfg = cfg.getCalibCfg();
    const double diameter = calib_cfg.getDiameter();
    cv::Point2d left_top = calib_cfg.getLeftTop();
    cv::Point2d right_top = calib_cfg.getRightTop();
    cv::Point2d left_bottom = calib_cfg.getLeftBottom();
    cv::Point2d right_bottom = calib_cfg.getRightBottom();
    auto imgsize = cfg.getSpecificCfg().getImgSize();

    if (calib_cfg.getRotation() > std::numbers::pi / 4.0) {
        std::swap(left_top.x, left_top.y);
        std::swap(right_bottom.x, right_bottom.y);
        std::swap(right_top.x, right_top.y);
        std::swap(left_bottom.x, left_bottom.y);
        std::swap(right_top, left_bottom);
        std::swap(imgsize.height, imgsize.width);
    }

    const auto veclen = [](const cv::Point2d vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y); };

    const cv::Point2d top_x_shift = right_top - left_top;
    const int top_cols = _hp::iround(veclen(top_x_shift) / diameter) + 1;
    const cv::Point2d top_x_unit_shift = top_x_shift / (top_cols - 1);

    bool is_out_shift;
    if (left_top.x < top_x_unit_shift.x) {
        is_out_shift = false;
    } else {
        is_out_shift = true;
    }

    TMiCols micols{top_cols, top_cols};
    if (is_out_shift) {
        // Now the second row have one more intact MI than the first row
        if (left_top.x + top_x_unit_shift.x * top_cols < imgsize.width) {
            micols[1]++;
        }
    } else {
        // Now the second row have one less intact MI than the first row
        if (left_top.x + top_x_unit_shift.x * top_cols >= imgsize.width) {
            micols[1]--;
        }
    }

    const cv::Point2d left_y_shift = left_bottom - left_top;
    const double approx_y_unit_shift = diameter * std::numbers::sqrt3 / 2.0;
    const int left_y_rows = _hp::iround(veclen(left_y_shift) / approx_y_unit_shift) + 1;
    const cv::Point2d left_y_unit_shift = left_y_shift / (left_y_rows - 1);
    const int mirows = (int)(((double)imgsize.height - diameter / 2.0 - left_top.y) / left_y_unit_shift.y) + 1;

    const cv::Point2d right_y_shift = right_bottom - right_top;
    const cv::Point2d right_y_unit_shift = right_y_shift / (left_y_rows - 1);

    return {left_top, right_top, is_out_shift, left_y_unit_shift, right_y_unit_shift,
            mirows,   micols,    imgsize,      diameter,          calib_cfg.getRotation()};
}

Layout& Layout::upsample(const int factor) noexcept
{
    left_top_ *= factor;
    right_top_ *= factor;
    left_y_unit_shift_ *= factor;
    right_y_unit_shift_ *= factor;
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    upsample_ = factor;
    return *this;
}

cv::Point2d Layout::getMICenter(const int row, const int col) const noexcept
{
    cv::Point2d left = left_top_ + left_y_unit_shift_ * row;
    cv::Point2d right = right_top_ + right_y_unit_shift_ * row;
    cv::Point2d x_unit_shift = (right - left) / (micols_[0] - 1);
    cv::Point2d center = left + x_unit_shift * col;

    if (row % 2 == 1) {
        center -= x_unit_shift / 2.0 * isOutShiftSgn();
    }

    return center;
}

cv::Point2d Layout::getMICenter(const cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

void Layout::procImg_(const cv::Mat& src, cv::Mat& dst) const
{
    dst = src;

    const double rotation = getRotation();
    if (rotation > std::numbers::pi / 4.0) {
        cv::Mat transposed_src;
        cv::transpose(src, transposed_src);
        dst = std::move(transposed_src);
    }

    const int upsample = getUpsample();
    if (upsample != 1) [[likely]] {
        cv::Mat upsampled_src;
        cv::resize(dst, upsampled_src, {}, upsample, upsample, cv::INTER_CUBIC);
        dst = std::move(upsampled_src);
    }
}

cv::Mat Layout::procImg(const cv::Mat& src) const
{
    cv::Mat dst;
    procImg_(src, dst);
    return dst;
}

} // namespace tlct::_cfg::tspc
