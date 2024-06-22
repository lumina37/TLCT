#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts/layout.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/direction.hpp"

namespace tlct::cvt::_hp {

template <typename TLayout_, int LEN_TYPE_NUM_>
    requires tlct::cfg::concepts::CLayout<TLayout_>
class NeighborIdx_
{
public:
    static constexpr int LEN_TYPE_NUM = LEN_TYPE_NUM_;
    static constexpr int DEFAULT_INDEX = -1;

    // Typename alias
    using TLayout = TLayout_;

    // Constructor
    TLCT_API inline NeighborIdx_& operator=(const NeighborIdx_& rhs) noexcept = default;
    TLCT_API inline NeighborIdx_(const NeighborIdx_& rhs) noexcept = default;
    TLCT_API inline NeighborIdx_& operator=(NeighborIdx_&& rhs) noexcept = default;
    TLCT_API inline NeighborIdx_(NeighborIdx_&& rhs) noexcept = default;
    TLCT_API inline NeighborIdx_(cv::Point left, cv::Point right, cv::Point upleft, cv::Point upright,
                                 cv::Point downleft, cv::Point downright) noexcept
        : left_(left), right_(right), upleft_(upleft), upright_(upright), downleft_(downleft), downright_(downright){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline NeighborIdx_ fromLayoutAndIndex(const TLayout& layout,
                                                                         const cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline bool hasLeft() const noexcept { return left_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasRight() const noexcept { return right_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpLeft() const noexcept { return upleft_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpRight() const noexcept { return upright_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownLeft() const noexcept { return downleft_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownRight() const noexcept { return downright_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline cv::Point getLeft() const noexcept { return left_; };
    [[nodiscard]] TLCT_API inline cv::Point getRight() const noexcept { return right_; };
    [[nodiscard]] TLCT_API inline cv::Point getUpLeft() const noexcept { return upleft_; };
    [[nodiscard]] TLCT_API inline cv::Point getUpRight() const noexcept { return upright_; };
    [[nodiscard]] TLCT_API inline cv::Point getDownLeft() const noexcept { return downleft_; };
    [[nodiscard]] TLCT_API inline cv::Point getDownRight() const noexcept { return downright_; };

    template <Direction direction>
    [[nodiscard]] TLCT_API inline cv::Point2d getNeighbor() const noexcept
    {
        if constexpr (direction == Direction::LEFT)
            return getLeft();
        if constexpr (direction == Direction::RIGHT)
            return getRight();
        if constexpr (direction == Direction::UPLEFT)
            return getUpLeft();
        if constexpr (direction == Direction::UPRIGHT)
            return getUpRight();
        if constexpr (direction == Direction::DOWNLEFT)
            return getDownLeft();
        if constexpr (direction == Direction::DOWNRIGHT)
            return getDownRight();
    };

    template <Direction direction>
    [[nodiscard]] TLCT_API inline bool hasNeighbor() const noexcept
    {
        if constexpr (direction == Direction::LEFT)
            return hasLeft();
        if constexpr (direction == Direction::RIGHT)
            return hasRight();
        if constexpr (direction == Direction::UPLEFT)
            return hasUpLeft();
        if constexpr (direction == Direction::UPRIGHT)
            return hasUpRight();
        if constexpr (direction == Direction::DOWNLEFT)
            return hasDownLeft();
        if constexpr (direction == Direction::DOWNRIGHT)
            return hasDownRight();
    };

private:
    cv::Point left_;
    cv::Point right_;
    cv::Point upleft_;
    cv::Point upright_;
    cv::Point downleft_;
    cv::Point downright_;
};

template <typename TLayout, int LEN_TYPE_NUM>
    requires tlct::cfg::concepts::CLayout<TLayout>
NeighborIdx_<TLayout, LEN_TYPE_NUM>
NeighborIdx_<TLayout, LEN_TYPE_NUM>::fromLayoutAndIndex(const TLayout& layout, const cv::Point index) noexcept
{
    cv::Point left{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};
    cv::Point right{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};
    cv::Point upleft{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};
    cv::Point upright{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};
    cv::Point downleft{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};
    cv::Point downright{NeighborIdx_::DEFAULT_INDEX, NeighborIdx_::DEFAULT_INDEX};

    if (index.x > LEN_TYPE_NUM - 1) {
        left = {index.x - LEN_TYPE_NUM, index.y};
    }
    if (index.x < (layout.getMICols(index.y) - LEN_TYPE_NUM)) {
        right = {index.x + LEN_TYPE_NUM, index.y};
    }

    constexpr int type_shift = LEN_TYPE_NUM / 2;
    const int is_left_row = layout.isOutShift() ^ (index.y % 2 == 0); // this row is on the left side of up/down row
    const int udleft_xidx = index.x - type_shift - is_left_row;       // x index of the up/down-left MI
    const int udright_xidx = udleft_xidx + LEN_TYPE_NUM;              // x index of the up/down-right MI

    if (index.y > 0) {
        const int yidx = index.y - 1;
        if (udleft_xidx >= 0) {
            upleft = {udleft_xidx, yidx};
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            upright = {udright_xidx, yidx};
        }
    }

    if (index.y < (layout.getMIRows() - 1)) {
        const int yidx = index.y + 1;
        if (udleft_xidx >= 0) {
            downleft = {udleft_xidx, yidx};
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            downright = {udright_xidx, yidx};
        }
    }

    return {left, right, upleft, upright, downleft, downright};
}

} // namespace tlct::cvt::_hp
