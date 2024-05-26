#pragma once

#include <numbers>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::cfg::raytrix {

class Layout
{
public:
    using TCalibConfig = CalibConfig;

    TLCT_API Layout() noexcept
        : left_top_(), is_out_shift_(), x_unit_shift_(), y_unit_shift_(), mirows_(), micols_(), imgsize_(), diameter_(),
          radius_(), rotation_(), upsample_(1) {};
    TLCT_API Layout& operator=(const Layout& layout) noexcept = default;
    TLCT_API Layout(const Layout& layout) noexcept = default;
    TLCT_API Layout& operator=(Layout&& layout) noexcept = default;
    TLCT_API Layout(Layout&& layout) noexcept = default;
    TLCT_API Layout(cv::Point2d point, cv::Size imgsize, double diameter, double rotation) noexcept;

    [[nodiscard]] TLCT_API static Layout fromCfgAndImgsize(const TCalibConfig& cfg, cv::Size imgsize);

    TLCT_API Layout& upsample(int factor) noexcept;
    TLCT_API Layout& transpose() noexcept;

    [[nodiscard]] TLCT_API int getImgWidth() const noexcept;
    [[nodiscard]] TLCT_API int getImgHeight() const noexcept;
    [[nodiscard]] TLCT_API cv::Size getImgSize() const noexcept;
    [[nodiscard]] TLCT_API double getDiameter() const noexcept;
    [[nodiscard]] TLCT_API double getRadius() const noexcept;
    [[nodiscard]] TLCT_API double getRotation() const noexcept;
    [[nodiscard]] TLCT_API int getUpsample() const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API int getMIRows() const noexcept;
    [[nodiscard]] TLCT_API int getMICols(int row) const noexcept;
    [[nodiscard]] TLCT_API int getMIMaxCols() const noexcept;
    [[nodiscard]] TLCT_API int getMIMinCols() const noexcept;
    [[nodiscard]] TLCT_API bool isOutShift() const noexcept;
    [[nodiscard]] TLCT_API int isOutShiftSgn() const noexcept;

private:
    cv::Point2d left_top_;
    bool is_out_shift_;
    double x_unit_shift_;
    double y_unit_shift_;
    cv::Size imgsize_;
    cv::Vec2i mirows_;
    cv::Vec2i micols_;
    double diameter_;
    double radius_;
    double rotation_;
    int upsample_;
};

static_assert(concepts::CLayout<Layout>);

inline Layout::Layout(cv::Point2d point, const cv::Size imgsize, double diameter, double rotation) noexcept
    : imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0), rotation_(rotation), upsample_(1)
{
    if (rotation_ != 0.0) {
        transpose();
        std::swap(point.x, point.y);
    }

    x_unit_shift_ = diameter;
    y_unit_shift_ = diameter * std::numbers::sqrt3 / 2.0;

    const int point_yidx = (int)((point.y - radius_) / y_unit_shift_);
    const double left_x = point.x - std::floor((point.x - x_unit_shift_ / 2.0) / x_unit_shift_) * x_unit_shift_;
    if (point_yidx % 2 == 0) {
        left_top_.x = left_x;
        if (left_top_.x > diameter_) {
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
    left_top_.y = point.y - std::floor((point.y - y_unit_shift_ / 2.0) / y_unit_shift_) * y_unit_shift_;

    micols_[0] = (int)(((double)imgsize_.width - left_top_.x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    micols_[1] = (int)(((double)imgsize_.width - getMICenter(1, 0).x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    mirows_[0] = (int)(((double)imgsize_.height - left_top_.y - y_unit_shift_ / 2.0) / y_unit_shift_) + 1;
    mirows_[1] = mirows_[0];
}

inline Layout Layout::fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize)
{
    const cv::Point2d center = cv::Point2d(imgsize) / 2.0;
    const cv::Point2d point{center.x + cfg.offset_.x, center.y - cfg.offset_.y};
    return {point, imgsize, cfg.diameter_, cfg.rotation_};
}

inline Layout& Layout::upsample(int factor) noexcept
{
    left_top_ *= factor;
    x_unit_shift_ *= factor;
    y_unit_shift_ *= factor;
    imgsize_ *= factor;
    diameter_ *= factor;
    radius_ *= factor;
    upsample_ = factor;
    return *this;
}

inline Layout& Layout::transpose() noexcept
{
    std::swap(left_top_.x, left_top_.y);
    std::swap(x_unit_shift_, y_unit_shift_);
    std::swap(micols_, mirows_);
    std::swap(imgsize_.height, imgsize_.width);
    return *this;
}

inline int Layout::getImgWidth() const noexcept { return imgsize_.width; }

inline int Layout::getImgHeight() const noexcept { return imgsize_.height; }

inline cv::Size Layout::getImgSize() const noexcept { return imgsize_; }

inline double Layout::getDiameter() const noexcept { return diameter_; }

inline double Layout::getRadius() const noexcept { return radius_; }

inline double Layout::getRotation() const noexcept { return rotation_; }

inline int Layout::getUpsample() const noexcept { return upsample_; }

inline cv::Point2d Layout::getMICenter(const int row, const int col) const noexcept
{
    cv::Point2d center{left_top_.x + x_unit_shift_ * col, left_top_.y + y_unit_shift_ * row};
    if (row % 2 == 1) {
        center.x += x_unit_shift_ / 2.0 * (-isOutShiftSgn());
    }
    return center;
}

inline cv::Point2d Layout::getMICenter(const cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

inline int Layout::getMIRows() const noexcept { return mirows_[0]; }

inline int Layout::getMICols(int row) const noexcept { return micols_[row % micols_.channels]; }

inline int Layout::getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); }

inline int Layout::getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); }

inline bool Layout::isOutShift() const noexcept { return is_out_shift_; }

inline int Layout::isOutShiftSgn() const noexcept { return (int)(isOutShift()) * 2 - 1; }

TLCT_API inline void procImg_(const Layout& layout, const cv::Mat& src, cv::Mat& dst)
{
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

TLCT_API inline cv::Mat procImg(const Layout& layout, const cv::Mat& src)
{
    cv::Mat dst;
    procImg_(layout, src, dst);
    return dst;
}

} // namespace tlct::cfg::raytrix