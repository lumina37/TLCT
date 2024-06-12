#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cvt::tspc::_hp {

namespace tcfg = tlct::cfg::tspc;
using namespace tlct::cvt::_hp;

class NeibMIIndices
{
public:
    static constexpr int DEFAULT_INDEX = -1;

    // Constructor
    TLCT_API inline NeibMIIndices& operator=(const NeibMIIndices& rhs) noexcept = default;
    TLCT_API inline NeibMIIndices(const NeibMIIndices& rhs) noexcept = default;
    TLCT_API inline NeibMIIndices& operator=(NeibMIIndices&& rhs) noexcept = default;
    TLCT_API inline NeibMIIndices(NeibMIIndices&& rhs) noexcept = default;
    TLCT_API inline NeibMIIndices(cv::Point left, cv::Point right, cv::Point upleft, cv::Point upright,
                                  cv::Point downleft, cv::Point downright) noexcept
        : left_(left), right_(right), upleft_(upleft), upright_(upright), downleft_(downleft), downright_(downright){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline NeibMIIndices fromLayoutAndIndex(const tcfg::Layout& layout,
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

NeibMIIndices NeibMIIndices::fromLayoutAndIndex(const tcfg::Layout& layout, const cv::Point index) noexcept
{
    cv::Point left{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};
    cv::Point right{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};
    cv::Point upleft{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};
    cv::Point upright{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};
    cv::Point downleft{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};
    cv::Point downright{NeibMIIndices::DEFAULT_INDEX, NeibMIIndices::DEFAULT_INDEX};

    if (index.x > 0) {
        left = {index.x - 1, index.y};
    }
    if (index.x < (layout.getMICols(index.y) - 1)) {
        right = {index.x + 1, index.y};
    }

    if (index.y > 0) {
        if (layout.isOutShift()) {
            upright = {index.x, index.y - 1};
            if (index.x > 0) {
                upleft = {index.x - 1, index.y - 1};
            }
        } else {
            upleft = {index.x, index.y - 1};
            if (index.x < (layout.getMICols(index.y - 1) - 1)) {
                upright = {index.x + 1, index.y - 1};
            }
        }
    }

    if (index.y < (layout.getMIRows() - 1)) {
        if (layout.isOutShift()) {
            downright = {index.x, index.y + 1};
            if (index.x > 0) {
                downleft = {index.x - 1, index.y + 1};
            }
        } else {
            downleft = {index.x, index.y + 1};
            if (index.x < (layout.getMICols(index.y + 1) - 1)) {
                downright = {index.x + 1, index.y + 1};
            }
        }
    }

    return {left, right, upleft, upright, downleft, downright};
}

} // namespace tlct::cvt::tspc::_hp
