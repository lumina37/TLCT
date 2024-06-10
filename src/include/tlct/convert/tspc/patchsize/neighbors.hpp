#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cvt::tspc::_hp {

namespace tcfg = tlct::cfg::tspc;

class NeibMIIndices
{
public:
    TLCT_API static constexpr int DEFAULT_INDEX = -1;
    TLCT_API static constexpr int NEIB_NUM = 6;

    TLCT_API inline NeibMIIndices() noexcept
        : left(DEFAULT_INDEX, DEFAULT_INDEX), right(DEFAULT_INDEX, DEFAULT_INDEX), upleft(DEFAULT_INDEX, DEFAULT_INDEX),
          upright(DEFAULT_INDEX, DEFAULT_INDEX), downleft(DEFAULT_INDEX, DEFAULT_INDEX),
          downright(DEFAULT_INDEX, DEFAULT_INDEX)
    {
    }

    [[nodiscard]] TLCT_API inline bool hasLeft() const noexcept { return left.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasRight() const noexcept { return right.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpLeft() const noexcept { return upleft.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasUpRight() const noexcept { return upright.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownLeft() const noexcept { return downleft.x != DEFAULT_INDEX; }
    [[nodiscard]] TLCT_API inline bool hasDownRight() const noexcept { return downright.x != DEFAULT_INDEX; }

    cv::Point left;
    cv::Point right;
    cv::Point upleft;
    cv::Point upright;
    cv::Point downleft;
    cv::Point downright;
};

static inline NeibMIIndices getNeibMIIndices(const tcfg::Layout& layout, const cv::Point index) noexcept
{
    NeibMIIndices indices{};

    if (index.x > 0) {
        indices.left = {index.x - 1, index.y};
    }
    if (index.x < (layout.getMICols(index.y) - 1)) {
        indices.right = {index.x + 1, index.y};
    }

    if (index.y > 0) {
        if (layout.isOutShift()) {
            indices.upright = {index.x, index.y - 1};
            if (index.x > 0) {
                indices.upleft = {index.x - 1, index.y - 1};
            }
        } else {
            indices.upleft = {index.x, index.y - 1};
            if (index.x < (layout.getMICols(index.y - 1) - 1)) {
                indices.upright = {index.x + 1, index.y - 1};
            }
        }
    }

    if (index.y < (layout.getMIRows() - 1)) {
        if (layout.isOutShift()) {
            indices.downright = {index.x, index.y + 1};
            if (index.x > 0) {
                indices.downleft = {index.x - 1, index.y + 1};
            }
        } else {
            indices.downleft = {index.x, index.y + 1};
            if (index.x < (layout.getMICols(index.y + 1) - 1)) {
                indices.downright = {index.x + 1, index.y + 1};
            }
        }
    }

    return indices;
}

} // namespace tlct::cvt::tspc::_hp
