#pragma once

#include <array>
#include <numbers>

#include <opencv2/imgproc.hpp>
#include <toml++/toml.hpp>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cfg::tspc {

class Layout
{
public:
    static constexpr bool IS_KEPLER = true;

    // Typename alias
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API inline Layout() noexcept
        : imgsize_(), raw_imgsize_(), diameter_(), radius_(), transpose_(), left_top_(), right_top_(), is_out_shift_(),
          left_y_unit_shift_(), right_y_unit_shift_(), mirows_(), micols_(), upsample_(1){};
    TLCT_API inline Layout(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(cv::Size imgsize, double diameter, bool transpose, cv::Point2d left_top,
                           cv::Point2d right_top, cv::Point2d left_bottom, cv::Point2d right_bottom) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline Layout fromToml(const toml::table& table);

    // Non-const methods
    TLCT_API inline Layout& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline cv::Size getRawImgSize() const noexcept { return raw_imgsize_; };
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline bool isTranspose() const noexcept { return transpose_; };
    [[nodiscard]] TLCT_API static consteval inline bool isKepler() noexcept { return IS_KEPLER; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return mirows_; };
    [[nodiscard]] TLCT_API inline int getMICols(const int row) const noexcept { return micols_[row % micols_.size()]; };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return is_out_shift_; };
    [[nodiscard]] TLCT_API inline int isOutShiftSgn() const noexcept { return _hp::sgn(isOutShift()); };

    TLCT_API inline void processInto(const cv::Mat& src, cv::Mat& dst) const;

private:
    cv::Size imgsize_;
    cv::Size raw_imgsize_;
    double diameter_;
    double radius_;
    bool transpose_;
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_y_unit_shift_;
    cv::Point2d right_y_unit_shift_;
    TMiCols micols_{};
    int mirows_;
    int upsample_;
    bool is_out_shift_;
};

Layout Layout::fromToml(const toml::table& table)
{
    const auto node2point = [](toml::node_view<const toml::node> node) {
        return cv::Point2d{node[0].value<double>().value(), node[1].value<double>().value()};
    };

    cv::Size imgsize{table["width"].value<int>().value(), table["height"].value<int>().value()};
    const double diameter = table["diameter"].value<double>().value();
    const bool transpose = table["transpose"].value_or(false);
    const cv::Point2d left_top = node2point(table["ltop"]);
    const cv::Point2d right_top = node2point(table["rtop"]);
    const cv::Point2d left_bottom = node2point(table["lbot"]);
    const cv::Point2d right_bottom = node2point(table["rbot"]);

    return {imgsize, diameter, transpose, left_top, right_top, left_bottom, right_bottom};
}

Layout& Layout::upsample(int factor) noexcept
{
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

cv::Point2d Layout::getMICenter(int row, int col) const noexcept
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

cv::Point2d Layout::getMICenter(cv::Point index) const noexcept { return getMICenter(index.y, index.x); }

void Layout::processInto(const cv::Mat& src, cv::Mat& dst) const
{
    dst = src;

    if (isTranspose()) {
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

Layout::Layout(cv::Size imgsize, double diameter, bool transpose, cv::Point2d left_top, cv::Point2d right_top,
               cv::Point2d left_bottom, cv::Point2d right_bottom) noexcept
    : raw_imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0), transpose_(transpose), upsample_(1)
{
    if (transpose) {
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

    const auto veclen = [](const cv::Point2d vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y); };

    const cv::Point2d top_x_shift = right_top - left_top;
    const int top_cols = _hp::iround(veclen(top_x_shift) / diameter) + 1;
    const cv::Point2d top_x_unit_shift = top_x_shift / (top_cols - 1);

    if (left_top.x < top_x_unit_shift.x) {
        is_out_shift_ = false;
    } else {
        is_out_shift_ = true;
    }

    micols_ = {top_cols, top_cols};
    if (is_out_shift_) {
        // Now the second row have one more intact MI than the first row
        const double mi_1_0_x = left_top.x - top_x_unit_shift.x / 2.0;
        if (mi_1_0_x + top_x_unit_shift.x * top_cols < imgsize.width) {
            micols_[1]++;
        }
    } else {
        // Now the second row have one less intact MI than the first row
        const double mi_1_0_x = left_top.x + top_x_unit_shift.x / 2.0;
        if (mi_1_0_x + top_x_unit_shift.x * top_cols >= imgsize.width) {
            micols_[1]--;
        }
    }

    const cv::Point2d left_y_shift = left_bottom - left_top;
    const double approx_y_unit_shift = diameter * std::numbers::sqrt3 / 2.0;
    const int left_y_rows = _hp::iround(veclen(left_y_shift) / approx_y_unit_shift) + 1;
    left_y_unit_shift_ = left_y_shift / (left_y_rows - 1);
    mirows_ = (int)(((double)imgsize.height - diameter / 2.0 - left_top.y) / left_y_unit_shift_.y) + 1;

    const cv::Point2d right_y_shift = right_bottom - right_top;
    right_y_unit_shift_ = right_y_shift / (left_y_rows - 1);
}

} // namespace tlct::_cfg::tspc
