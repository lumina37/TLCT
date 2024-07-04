#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts/layout.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/convert/helper/direction.hpp"

namespace tlct::cvt::_hp {

template <typename TLayout_, int LEN_TYPE_NUM_>
    requires tlct::cfg::concepts::CLayout<TLayout_>
class Neighbors_
{
public:
    static constexpr int LEN_TYPE_NUM = LEN_TYPE_NUM_;
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr double DEFAULT_COORD = -1.0;

    // Typename alias
    using TLayout = TLayout_;

    // Constructor
    Neighbors_() = delete;
    TLCT_API inline Neighbors_& operator=(const Neighbors_& rhs) noexcept = default;
    TLCT_API inline Neighbors_(const Neighbors_& rhs) noexcept = default;
    TLCT_API inline Neighbors_& operator=(Neighbors_&& rhs) noexcept = default;
    TLCT_API inline Neighbors_(Neighbors_&& rhs) noexcept = default;
    TLCT_API inline Neighbors_(cv::Point self_idx, cv::Point left_idx, cv::Point right_idx, cv::Point upleft_idx,
                               cv::Point upright_idx, cv::Point downleft_idx, cv::Point downright_idx,
                               cv::Point2d self_pt, cv::Point2d left_pt, cv::Point2d right_pt, cv::Point2d upleft_pt,
                               cv::Point2d upright_pt, cv::Point2d downleft_pt, cv::Point2d downright_pt) noexcept
        : self_idx_(self_idx), left_idx_(left_idx), right_idx_(right_idx), upleft_idx_(upleft_idx),
          upright_idx_(upright_idx), downleft_idx_(downleft_idx), downright_idx_(downright_idx), self_pt_(self_pt),
          left_pt_(left_pt), right_pt_(right_pt), upleft_pt_(upleft_pt), upright_pt_(upright_pt),
          downleft_pt_(downleft_pt), downright_pt_(downright_pt){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline Neighbors_ fromLayoutAndIndex(const TLayout& layout,
                                                                       const cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline bool hasLeft() const noexcept { return left_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasRight() const noexcept { return right_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpLeft() const noexcept { return upleft_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpRight() const noexcept { return upright_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownLeft() const noexcept { return downleft_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownRight() const noexcept { return downright_idx_.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline cv::Point getSelfIdx() const noexcept { return self_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getLeftIdx() const noexcept { return left_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getRightIdx() const noexcept { return right_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getUpLeftIdx() const noexcept { return upleft_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getUpRightIdx() const noexcept { return upright_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getDownLeftIdx() const noexcept { return downleft_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point getDownRightIdx() const noexcept { return downright_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getSelfPt() const noexcept { return self_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getLeftPt() const noexcept { return left_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getRightPt() const noexcept { return right_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getUpLeftPt() const noexcept { return upleft_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getUpRightPt() const noexcept { return upright_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getDownLeftPt() const noexcept { return downleft_pt_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getDownRightPt() const noexcept { return downright_pt_; };

    template <Direction direction>
    [[nodiscard]] TLCT_API inline cv::Point getNeighborIdx() const noexcept
    {
        if constexpr (direction == Direction::LEFT)
            return getLeftIdx();
        if constexpr (direction == Direction::RIGHT)
            return getRightIdx();
        if constexpr (direction == Direction::UPLEFT)
            return getUpLeftIdx();
        if constexpr (direction == Direction::UPRIGHT)
            return getUpRightIdx();
        if constexpr (direction == Direction::DOWNLEFT)
            return getDownLeftIdx();
        if constexpr (direction == Direction::DOWNRIGHT)
            return getDownRightIdx();
    };

    template <Direction direction>
    [[nodiscard]] TLCT_API inline cv::Point2d getNeighborPt() const noexcept
    {
        if constexpr (direction == Direction::LEFT)
            return getLeftPt();
        if constexpr (direction == Direction::RIGHT)
            return getRightPt();
        if constexpr (direction == Direction::UPLEFT)
            return getUpLeftPt();
        if constexpr (direction == Direction::UPRIGHT)
            return getUpRightPt();
        if constexpr (direction == Direction::DOWNLEFT)
            return getDownLeftPt();
        if constexpr (direction == Direction::DOWNRIGHT)
            return getDownRightPt();
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
    cv::Point self_idx_;
    cv::Point left_idx_;
    cv::Point right_idx_;
    cv::Point upleft_idx_;
    cv::Point upright_idx_;
    cv::Point downleft_idx_;
    cv::Point downright_idx_;
    cv::Point2d self_pt_;
    cv::Point2d left_pt_;
    cv::Point2d right_pt_;
    cv::Point2d upleft_pt_;
    cv::Point2d upright_pt_;
    cv::Point2d downleft_pt_;
    cv::Point2d downright_pt_;
};

template <typename TLayout, int LEN_TYPE_NUM>
    requires tlct::cfg::concepts::CLayout<TLayout>
Neighbors_<TLayout, LEN_TYPE_NUM> Neighbors_<TLayout, LEN_TYPE_NUM>::fromLayoutAndIndex(const TLayout& layout,
                                                                                        const cv::Point index) noexcept
{
    cv::Point left_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point right_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point upleft_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point upright_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point downleft_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point downright_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};

    cv::Point2d self_pt = layout.getMICenter(index);
    cv::Point2d left_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d right_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d upleft_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d upright_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d downleft_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d downright_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};

    if (index.x > LEN_TYPE_NUM - 1) {
        left_idx = {index.x - LEN_TYPE_NUM, index.y};
        left_pt = layout.getMICenter(left_idx);
    }
    if (index.x < (layout.getMICols(index.y) - LEN_TYPE_NUM)) {
        right_idx = {index.x + LEN_TYPE_NUM, index.y};
        right_pt = layout.getMICenter(right_idx);
    }

    constexpr int type_shift = LEN_TYPE_NUM / 2;
    const int is_left_row = layout.isOutShift() ^ (index.y % 2 == 0); // this row is on the left side of up/down row
    const int udleft_xidx = index.x - type_shift - is_left_row;       // x index of the up/down-left MI
    const int udright_xidx = udleft_xidx + LEN_TYPE_NUM;              // x index of the up/down-right MI

    if (index.y > 0) {
        const int yidx = index.y - 1;
        if (udleft_xidx >= 0) {
            upleft_idx = {udleft_xidx, yidx};
            upleft_pt = layout.getMICenter(upleft_idx);
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            upright_idx = {udright_xidx, yidx};
            upright_pt = layout.getMICenter(upright_idx);
        }
    }

    if (index.y < (layout.getMIRows() - 1)) {
        const int yidx = index.y + 1;
        if (udleft_xidx >= 0) {
            downleft_idx = {udleft_xidx, yidx};
            downleft_pt = layout.getMICenter(downleft_idx);
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            downright_idx = {udright_xidx, yidx};
            downright_pt = layout.getMICenter(downright_idx);
        }
    }

    return {index,   left_idx, right_idx, upleft_idx, upright_idx, downleft_idx, downright_idx,
            self_pt, left_pt,  right_pt,  upleft_pt,  upright_pt,  downleft_pt,  downright_pt};
}

} // namespace tlct::cvt::_hp
