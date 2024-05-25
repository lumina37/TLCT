#pragma once

#include <numbers>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/helper/static_math.hpp"

namespace tlct::cfg::tspc::inline v2 {

struct TLCT_API BorderCheckList {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

class Layout
{
public:
    TLCT_API Layout() noexcept
        : left_top_(), right_top_(), left_bottom_(), is_out_shift_(), x_unit_shift_(), y_unit_shift_(), mirows_(),
          micols_(), imgsize_(), diameter_(), radius_(), rotation_(), upsample_(1) {};
    TLCT_API Layout& operator=(const Layout& layout) noexcept = default;
    TLCT_API Layout(const Layout& layout) noexcept = default;
    TLCT_API Layout& operator=(Layout&& layout) noexcept = default;
    TLCT_API Layout(Layout&& layout) noexcept = default;
    TLCT_API Layout(cv::Point2d left_top, cv::Point2d right_top, cv::Point2d left_bottom, cv::Size imgsize, int mirows,
                    int micols, double diameter, double rotation);

    [[nodiscard]] TLCT_API static Layout fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize);

    TLCT_API Layout& upsample(int factor) noexcept;
    TLCT_API Layout& transpose();

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

    template <BorderCheckList checklist = {true, true, true, true}>
    [[nodiscard]] bool isMIBroken(cv::Point2d micenter) const noexcept;

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

inline Layout::Layout(const cv::Point2d left_top, const cv::Point2d right_top, const cv::Point2d left_bottom,
                      cv::Size imgsize, int mirows, int micols, double diameter, double rotation)
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

inline Layout Layout::fromCfgAndImgsize(const CalibConfig& cfg, cv::Size imgsize)
{
    return {cfg.left_top_, cfg.right_top_, cfg.left_bottom_, imgsize,
            cfg.rows_,     cfg.cols_,      cfg.diameter_,    cfg.rotation_};
}

inline Layout& Layout::upsample(int factor) noexcept
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

inline Layout& Layout::transpose()
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

inline int Layout::getImgWidth() const noexcept { return imgsize_.width; }

inline int Layout::getImgHeight() const noexcept { return imgsize_.height; }

inline cv::Size Layout::getImgSize() const noexcept { return imgsize_; }

inline double Layout::getDiameter() const noexcept { return diameter_; }

inline double Layout::getRadius() const noexcept { return radius_; }

inline double Layout::getRotation() const noexcept { return rotation_; }

inline int Layout::getUpsample() const noexcept { return upsample_; }

inline cv::Point2d Layout::getMICenter(const int row, const int col) const noexcept
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

inline cv::Point2d Layout::getMICenter(const cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

inline int Layout::getMIRows() const noexcept { return mirows_[0]; }

inline int Layout::getMICols(int row) const noexcept { return micols_[row % micols_.channels]; }

inline int Layout::getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); }

inline int Layout::getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); }

inline bool Layout::isOutShift() const noexcept { return is_out_shift_; }

template <BorderCheckList checklist>
inline bool Layout::isMIBroken(const cv::Point2d micenter) const noexcept
{
    if (checklist.up && micenter.y < radius_) {
        return true;
    }
    if (checklist.down && micenter.y > imgsize_.height - radius_) {
        return true;
    }
    if (checklist.left && micenter.x < radius_) {
        return true;
    }
    if (checklist.right && micenter.x > imgsize_.width - radius_) {
        return true;
    }
    return false;
}

template <BorderCheckList checklist>
inline cv::Rect restrictToImgBorder(const Layout& layout, const cv::Rect area) noexcept
{
    cv::Rect modarea{area};

    if (checklist.up && area.y < 0) {
        modarea.y = 0;
    }
    if (checklist.down && area.y + area.height > layout.getImgHeight()) {
        modarea.height = layout.getImgHeight() - modarea.y;
    }
    if (checklist.left && area.x < 0) {
        modarea.x = 0;
    }
    if (checklist.right && area.x + area.width > layout.getImgWidth()) {
        modarea.width = layout.getImgWidth() - modarea.x;
    }

    return modarea;
}

template <BorderCheckList checklist>
inline std::vector<cv::Range> restrictToImgBorder(const Layout& layout, const std::vector<cv::Range>& ranges) noexcept
{
    std::vector<cv::Range> modranges{ranges};

    if (checklist.up && ranges[0].start < 0) {
        modranges[0].start = 0;
    }
    if (checklist.down && ranges[0].end > layout.getImgHeight()) {
        modranges[0].end = layout.getImgHeight();
    }
    if (checklist.left && ranges[1].start < 0) {
        modranges[1].start = 0;
    }
    if (checklist.right && ranges[1].end > layout.getImgWidth()) {
        modranges[1].end = layout.getImgWidth();
    }

    return modranges;
}

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

} // namespace tlct::cfg::tspc::inline v2