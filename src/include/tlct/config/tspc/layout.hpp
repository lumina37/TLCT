#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts/layout.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cfg::tspc {

class Layout
{
public:
    // Typename alias
    using TCalibConfig = CalibConfig;

    // Constructor
    TLCT_API inline Layout() noexcept
        : left_top_(), right_top_(), left_bottom_(), is_out_shift_(), x_unit_shift_(), y_unit_shift_(), mirows_(),
          micols_(), imgsize_(), diameter_(), radius_(), rotation_(), upsample_(1){};
    TLCT_API inline Layout& operator=(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(cv::Point2d left_top, cv::Point2d right_top, cv::Point2d left_bottom, cv::Size imgsize,
                           int mirows, int micols, double diameter, double rotation) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline Layout fromCfgAndImgsize(const TCalibConfig& cfg, cv::Size imgsize);

    // Non-const methods
    TLCT_API inline Layout& upsample(int factor) noexcept;
    TLCT_API inline Layout& transpose() noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline double getRotation() const noexcept { return rotation_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return mirows_[0]; };
    [[nodiscard]] TLCT_API inline int getMICols(int row) const noexcept { return micols_[row % micols_.channels]; };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return is_out_shift_; };
    [[nodiscard]] TLCT_API inline int isOutShiftSgn() const noexcept { return (int)(isOutShift()) * 2 - 1; };
    ;

    // Utils
    TLCT_API static inline void procImg_(const Layout& layout, const cv::Mat& src, cv::Mat& dst);
    [[nodiscard]] TLCT_API static inline cv::Mat procImg(const Layout& layout, const cv::Mat& src);

private:
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_bottom_;
    bool is_out_shift_;
    cv::Point2d x_unit_shift_;
    cv::Point2d y_unit_shift_;
    cv::Size imgsize_;
    cv::Vec2i mirows_;
    cv::Vec2i micols_;
    double diameter_;
    double radius_;
    double rotation_;
    int upsample_;
};

static_assert(concepts::CLayout<Layout>);

Layout::Layout(const cv::Point2d left_top, const cv::Point2d right_top, const cv::Point2d left_bottom, cv::Size imgsize,
               int mirows, int micols, double diameter, double rotation) noexcept
    : left_top_(left_top), right_top_(right_top), left_bottom_(left_bottom), imgsize_(imgsize), mirows_(mirows, mirows),
      micols_(micols, micols), diameter_(diameter), radius_(diameter / 2.0), rotation_(rotation), upsample_(1)
{
    if (rotation_ != 0.0) {
        transpose();
    }

    cv::Point2d x_shift = right_top_ - left_top_;
    x_unit_shift_ = x_shift / (micols_[0] - 1);

    if (left_top_.x < x_unit_shift_.x) {
        is_out_shift_ = false;
    } else {
        is_out_shift_ = true;
    }

    if (is_out_shift_) {
        // Sometimes the second row may have one more intact MI than the first row
        if (left_top_.x + x_unit_shift_.x * micols_[1] < imgsize_.width) {
            micols_[1]++;
        }
    }

    cv::Point2d y_shift = left_bottom_ - left_top_;
    if (mirows_[0] % 2 == 0) {
        // `left_bottom` is in the `odd` row while `left_top` is in the `even` row
        // so we need to fix the `y_shift`
        if (is_out_shift_) {
            y_shift += x_unit_shift_ / 2.0;
        } else {
            y_shift -= x_unit_shift_ / 2.0;
        }
    }
    y_unit_shift_ = y_shift / (mirows_[0] - 1);
}

Layout Layout::fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize)
{
    return {cfg.left_top_, cfg.right_top_, cfg.left_bottom_, imgsize,
            cfg.rows_,     cfg.cols_,      cfg.diameter_,    cfg.rotation_};
}

Layout& Layout::upsample(int factor) noexcept
{
    left_top_ *= factor;
    right_top_ *= factor;
    left_bottom_ *= factor;
    x_unit_shift_ *= factor;
    y_unit_shift_ *= factor;
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    upsample_ = factor;
    return *this;
}

Layout& Layout::transpose() noexcept
{
    std::swap(left_top_.x, left_top_.y);
    std::swap(right_top_.x, right_top_.y);
    std::swap(left_bottom_.x, left_bottom_.y);
    std::swap(right_top_, left_bottom_);
    std::swap(x_unit_shift_.x, x_unit_shift_.y);
    std::swap(y_unit_shift_.x, y_unit_shift_.y);
    std::swap(x_unit_shift_, y_unit_shift_);
    std::swap(micols_, mirows_);
    std::swap(imgsize_.height, imgsize_.width);
    return *this;
}

cv::Point2d Layout::getMICenter(const int row, const int col) const noexcept
{
    cv::Point2d center = left_top_ + y_unit_shift_ * row + x_unit_shift_ * col;
    if (row % 2 == 1) {
        if (is_out_shift_) {
            center -= x_unit_shift_ / 2.0;
        } else {
            center += x_unit_shift_ / 2.0;
        }
    }
    return center;
}

cv::Point2d Layout::getMICenter(const cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

void Layout::procImg_(const Layout& layout, const cv::Mat& src, cv::Mat& dst)
{
    dst = src;

    const double rotation = layout.getRotation();
    if (rotation != 0.0) {
        cv::Mat transposed_src;
        cv::transpose(src, transposed_src);
        dst = std::move(transposed_src);
    }

    const int upsample = layout.getUpsample();
    if (upsample != 1) {
        cv::Mat upsampled_src;
        cv::resize(dst, upsampled_src, {}, upsample, upsample, cv::INTER_CUBIC);
        dst = std::move(upsampled_src);
    }
}

cv::Mat Layout::procImg(const Layout& layout, const cv::Mat& src)
{
    cv::Mat dst;
    procImg_(layout, src, dst);
    return dst;
}

} // namespace tlct::cfg::tspc