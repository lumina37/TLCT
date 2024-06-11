#pragma once

#include <numbers>

#include <opencv2/core.hpp>

#include "direction.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cvt {

class MatchPoints
{
public:
    // Constructor
    TLCT_API inline MatchPoints& operator=(const MatchPoints& rhs) noexcept = default;
    TLCT_API inline MatchPoints(const MatchPoints& rhs) noexcept = default;
    TLCT_API inline MatchPoints& operator=(MatchPoints&& rhs) noexcept = default;
    TLCT_API inline MatchPoints(MatchPoints&& rhs) noexcept = default;
    TLCT_API inline MatchPoints(cv::Point2d left, cv::Point2d right, cv::Point2d upleft, cv::Point2d upright,
                                cv::Point2d downleft, cv::Point2d downright) noexcept
        : left_(left), right_(right), upleft_(upleft), upright_(upright), downleft_(downleft), downright_(downright){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline MatchPoints fromDiameter(const double diameter,
                                                                  const double shift_factor = 0.5) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Point2d getLeft() const noexcept { return left_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getRight() const noexcept { return right_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getUpLeft() const noexcept { return upleft_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getUpRight() const noexcept { return upright_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getDownLeft() const noexcept { return downleft_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getDownRight() const noexcept { return downright_; };

    template <Direction direction>
    [[nodiscard]] TLCT_API inline cv::Point2d getMatchPoint() const noexcept
    {
        if constexpr (direction & Direction::LEFT)
            return getLeft();
        if constexpr (direction & Direction::RIGHT)
            return getRight();
        if constexpr (direction & Direction::UPLEFT)
            return getUpLeft();
        if constexpr (direction & Direction::UPRIGHT)
            return getUpRight();
        if constexpr (direction & Direction::DOWNLEFT)
            return getDownLeft();
        if constexpr (direction & Direction::DOWNRIGHT)
            return getDownRight();
    };

private:
    cv::Point2d left_;
    cv::Point2d right_;
    cv::Point2d upleft_;
    cv::Point2d upright_;
    cv::Point2d downleft_;
    cv::Point2d downright_;
};

MatchPoints MatchPoints::fromDiameter(const double diameter, const double shift_factor) noexcept
{
    const double radius = diameter / 2.0;
    const double radial_shift = radius * shift_factor;
    const double x_shift = radial_shift / 2.0;
    const double y_shift = x_shift * std::numbers::sqrt3;

    cv::Point2d left = {-radial_shift, 0.0};
    cv::Point2d right = {radial_shift, 0.0};
    cv::Point2d upleft = {-x_shift, -y_shift};
    cv::Point2d upright = {x_shift, -y_shift};
    cv::Point2d downleft = {-x_shift, y_shift};
    cv::Point2d downright = {x_shift, y_shift};

    return {left, right, upleft, upright, downleft, downright};
}

template <double amp>
class MatchSteps_
{
public:
    static constexpr double X_UNIT_STEP = 0.5 * amp;
    static constexpr double Y_UNIT_STEP = std::numbers::sqrt3 / 2.0 * amp;
};

} // namespace tlct::cvt
