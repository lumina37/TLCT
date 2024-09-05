#pragma once

#include <array>
#include <numbers>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "param.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/static_math.hpp"

namespace tlct::_cfg::raytrix {

namespace rgs = std::ranges;

class Layout
{
public:
    // Typename alias
    using TParamConfig = ParamConfig;
    using TIdx2Type = std::array<std::array<int, LEN_TYPE_NUM>, 2>;

    // Constructor
    TLCT_API inline Layout() noexcept
        : left_top_(), is_out_shift_(), x_unit_shift_(), y_unit_shift_(), imgsize_(), mirows_(), micols_(), diameter_(),
          radius_(), rotation_(), upsample_(1){};
    TLCT_API inline Layout& operator=(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout(const Layout& rhs) noexcept = default;
    TLCT_API inline Layout& operator=(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(Layout&& rhs) noexcept = default;
    TLCT_API inline Layout(cv::Point2d left_top, bool is_out_shift, double x_unit_shift, double y_unit_shift,
                           cv::Size imgsize, int mirows, cv::Vec2i micols, TIdx2Type idx2type, double diameter,
                           double rotation) noexcept
        : left_top_(left_top), is_out_shift_(is_out_shift), x_unit_shift_(x_unit_shift), y_unit_shift_(y_unit_shift),
          imgsize_(imgsize), mirows_(mirows), micols_(micols), idx2type_(idx2type), diameter_(diameter),
          radius_(diameter / 2.0), rotation_(rotation), upsample_(1){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline Layout fromParamConfig(const TParamConfig& cfg);

    // Non-const methods
    TLCT_API inline Layout& upsample(const int factor) noexcept;
    TLCT_API inline Layout& transpose() noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline int getImgWidth() const noexcept { return imgsize_.width; };
    [[nodiscard]] TLCT_API inline int getImgHeight() const noexcept { return imgsize_.height; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline int getMIType(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIType(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRadius() const noexcept { return radius_; };
    [[nodiscard]] TLCT_API inline double getRotation() const noexcept { return rotation_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline cv::Point2d getMICenter(cv::Point index) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIRows() const noexcept { return mirows_; };
    [[nodiscard]] TLCT_API inline int getMICols(const int row) const noexcept
    {
        return micols_[row % micols_.channels];
    };
    [[nodiscard]] TLCT_API inline int getMIMaxCols() const noexcept { return std::max(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline int getMIMinCols() const noexcept { return std::min(micols_[0], micols_[1]); };
    [[nodiscard]] TLCT_API inline bool isOutShift() const noexcept { return is_out_shift_; };
    [[nodiscard]] TLCT_API inline int isOutShiftSgn() const noexcept { return _hp::sgn(isOutShift()); };

    TLCT_API inline void procImg_(const cv::Mat& src, cv::Mat& dst) const;
    [[nodiscard]] TLCT_API inline cv::Mat procImg(const cv::Mat& src) const;

private:
    cv::Point2d left_top_;
    bool is_out_shift_;
    double x_unit_shift_;
    double y_unit_shift_;
    cv::Size imgsize_;
    int mirows_;
    cv::Vec2i micols_;
    TIdx2Type idx2type_;
    double diameter_;
    double radius_;
    double rotation_;
    int upsample_;
};

static_assert(concepts::CLayout<Layout>);

Layout Layout::fromParamConfig(const TParamConfig& cfg)
{
    const auto& calib_cfg = cfg.getCalibCfg();
    const double diameter = calib_cfg.getDiameter();
    const auto offset = calib_cfg.getOffset();
    auto imgsize = cfg.getSpecificCfg().getImgSize();

    cv::Point2d center_mi{imgsize.width / 2.0 + offset.x, imgsize.height / 2.0 - offset.y};

    if (calib_cfg.getRotation() > 1e-2) {
        std::swap(imgsize.height, imgsize.width);
        std::swap(center_mi.x, center_mi.y);
    }

    const double x_unit_shift = diameter;
    const double y_unit_shift = diameter * std::numbers::sqrt3 / 2.0;
    const double radius = diameter / 2.0;
    const int center_mi_xidx = (int)((center_mi.x - radius) / x_unit_shift);
    const int center_mi_yidx = (int)((center_mi.y - radius) / y_unit_shift);

    bool is_out_shift;
    cv::Point2d left_top;
    const double left_x = center_mi.x - x_unit_shift * center_mi_xidx;
    if (center_mi_yidx % 2 == 0) {
        left_top.x = left_x;
        if (left_top.x > diameter) {
            is_out_shift = true;
        } else {
            is_out_shift = false;
        }
    } else {
        if (left_x > diameter) {
            left_top.x = left_x - radius;
            is_out_shift = false;
        } else {
            left_top.x = left_x + radius;
            is_out_shift = true;
        }
    }
    left_top.y = center_mi.y - std::floor((center_mi.y - y_unit_shift / 2.0) / y_unit_shift) * y_unit_shift;

    cv::Vec2i micols;
    const double mi_1_0_x = left_top.x - x_unit_shift / 2.0 * _hp::sgn(is_out_shift);
    micols[0] = (int)(((double)imgsize.width - left_top.x - x_unit_shift / 2.0) / x_unit_shift) + 1;
    micols[1] = (int)(((double)imgsize.width - mi_1_0_x - x_unit_shift / 2.0) / x_unit_shift) + 1;
    const int mirows = (int)(((double)imgsize.height - left_top.y - y_unit_shift / 2.0) / y_unit_shift) + 1;

    TIdx2Type idx2type;
    const bool is_odd_yidx = center_mi_yidx % 2;
    for (const int type : rgs::views::iota(0, LEN_TYPE_NUM)) {
        const int ofs = calib_cfg.getLenOffsets()[type];
        const int idx = (center_mi_xidx + ofs + LEN_TYPE_NUM) % LEN_TYPE_NUM;
        idx2type[is_odd_yidx][idx] = type;
    }
    const bool is_another_row_on_left = is_odd_yidx ^ is_out_shift;
    for (const int idx : rgs::views::iota(0, LEN_TYPE_NUM)) {
        const int type = idx2type[is_odd_yidx][(idx + 2 - is_another_row_on_left) % 3];
        idx2type[!is_odd_yidx][idx] = type;
    }

    return {left_top, is_out_shift, x_unit_shift, y_unit_shift, imgsize,
            mirows,   micols,       idx2type,     diameter,     calib_cfg.getRotation()};
}

Layout& Layout::upsample(int factor) noexcept
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

Layout& Layout::transpose() noexcept { return *this; }

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

void Layout::procImg_(const cv::Mat& src, cv::Mat& dst) const
{
    dst = src;

    const double rotation = getRotation();
    if (rotation > 1e-2) {
        cv::Mat transposed_src;
        cv::transpose(src, transposed_src);
        dst = std::move(transposed_src);
    }

    const int upsample = getUpsample();
    if (upsample != 1) {
        cv::Mat upsampled_src;
        cv::resize(dst, upsampled_src, {}, upsample, upsample, cv::INTER_CUBIC);
        dst = std::move(upsampled_src);
    }
}

cv::Mat Layout::procImg(const cv::Mat& src) const
{
    cv::Mat dst;
    procImg_(src, dst);
    return std::move(dst);
}

} // namespace tlct::_cfg::raytrix
