#pragma once

#include <array>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>
#include <toml++/toml.hpp>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cfg::raytrix {

namespace rgs = std::ranges;

static constexpr int LEN_TYPE_NUM = 3;

class Layout
{
public:
    static constexpr bool IS_KEPLER = false;

    // Typename alias
    using TIdx2Type = std::array<std::array<int, LEN_TYPE_NUM>, 2>;
    using TMiCols = std::array<int, 2>;

    // Constructor
    TLCT_API inline Layout() noexcept
        : imgsize_(), raw_imgsize_(), diameter_(), radius_(), transpose_(), left_top_(), x_unit_shift_(),
          y_unit_shift_(), mirows_(), micols_(), idx2type_(), upsample_(1), is_out_shift_(){};
    TLCT_API inline Layout(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(cv::Size imgsize, double diameter, bool transpose, cv::Point2d offset) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static inline Layout fromToml(const toml::table& table);

    // Non-const methods
    TLCT_API inline Layout& upsample(int factor) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline cv::Size getRawImgSize() const noexcept { return raw_imgsize_; };
    [[nodiscard]] TLCT_API inline int getMIType(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIType(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline bool isTranspose() const noexcept { return transpose_; };
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
    double x_unit_shift_;
    double y_unit_shift_;
    int mirows_;
    TMiCols micols_;
    TIdx2Type idx2type_;
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
    const cv::Point2d offset = node2point(table["offset"]);

    return {imgsize, diameter, transpose, offset};
}

Layout& Layout::upsample(int factor) noexcept
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

int Layout::getMIType(int row, int col) const noexcept
{
    const int type = idx2type_[row % idx2type_.size()][col % LEN_TYPE_NUM];
    return type;
}

int Layout::getMIType(cv::Point index) const noexcept { return getMIType(index.y, index.x); }

cv::Point2d Layout::getMICenter(int row, int col) const noexcept
{
    cv::Point2d center{left_top_.x + x_unit_shift_ * col, left_top_.y + y_unit_shift_ * row};
    if (row % 2 == 1) {
        center.x -= x_unit_shift_ / 2.0 * isOutShiftSgn();
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

Layout::Layout(cv::Size imgsize, double diameter, bool transpose, cv::Point2d offset) noexcept
    : raw_imgsize_(imgsize), diameter_(diameter), radius_(diameter / 2.0), transpose_(transpose), upsample_(1)
{
    cv::Point2d center_mi{imgsize.width / 2.0 + offset.x, imgsize.height / 2.0 - offset.y};

    if (isTranspose()) {
        std::swap(imgsize.height, imgsize.width);
        std::swap(center_mi.x, center_mi.y);
    }
    imgsize_ = imgsize;

    x_unit_shift_ = diameter;
    y_unit_shift_ = diameter * std::numbers::sqrt3 / 2.0;
    const int center_mi_xidx = (int)((center_mi.x - radius_) / x_unit_shift_);
    const int center_mi_yidx = (int)((center_mi.y - radius_) / y_unit_shift_);

    const double left_x = center_mi.x - x_unit_shift_ * center_mi_xidx;
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

    const double mi_1_0_x = left_top_.x - x_unit_shift_ / 2.0 * _hp::sgn(is_out_shift_);
    micols_[0] = (int)(((double)imgsize.width - left_top_.x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    micols_[1] = (int)(((double)imgsize.width - mi_1_0_x - x_unit_shift_ / 2.0) / x_unit_shift_) + 1;
    mirows_ = (int)(((double)imgsize.height - left_top_.y - y_unit_shift_ / 2.0) / y_unit_shift_) + 1;

    const bool is_odd_yidx = center_mi_yidx % 2;
    for (const int type : rgs::views::iota(0, LEN_TYPE_NUM)) {
        idx2type_[is_odd_yidx][type] = type;
    }
    const bool is_another_row_on_left = is_odd_yidx ^ is_out_shift_;
    for (const int idx : rgs::views::iota(0, LEN_TYPE_NUM)) {
        const int type = idx2type_[is_odd_yidx][(idx + 2 - is_another_row_on_left) % 3];
        idx2type_[!is_odd_yidx][idx] = type;
    }
}

} // namespace tlct::_cfg::raytrix
