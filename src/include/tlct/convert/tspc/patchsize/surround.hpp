#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cvt::tspc::_hp {

namespace tcfg = tlct::cfg::tspc;

class SurroundMIIndices
{
public:
    TLCT_API static constexpr int DEFAULT_INDEX = -1;
    TLCT_API inline SurroundMIIndices()
        : left(DEFAULT_INDEX, DEFAULT_INDEX), right(DEFAULT_INDEX, DEFAULT_INDEX), upleft(DEFAULT_INDEX, DEFAULT_INDEX),
          upright(DEFAULT_INDEX, DEFAULT_INDEX), downleft(DEFAULT_INDEX, DEFAULT_INDEX),
          downright(DEFAULT_INDEX, DEFAULT_INDEX)
    {
    }

    cv::Point left;
    cv::Point right;
    cv::Point upleft;
    cv::Point upright;
    cv::Point downleft;
    cv::Point downright;
};

static inline SurroundMIIndices getSurroundMIIndices(const tcfg::Layout& layout, const cv::Point index)
{
    SurroundMIIndices indices{};

    if (index.x != 0) {
        indices.left = {index.x - 1, index.y};
    }
    if (index.y != 0) {
        if (layout.isOutShift()) {
            indices.upright = {index.x, index.y - 1};
            if (index.x != 0) {
                indices.upleft = {index.x - 1, index.y - 1};
            }
        } else {
            indices.upleft = {index.x, index.y - 1};
            if (index.x != (layout.getMICols(index.y - 1) - 1)) {
                indices.upright = {index.x + 1, index.y - 1};
            }
        }
    }
    if (index.y != layout.getMIRows() - 1) {
        if (layout.isOutShift()) {
            indices.downright = {index.x, index.y + 1};
            if (index.x != 0) {
                indices.downleft = {index.x - 1, index.y + 1};
            }
        } else {
            indices.downleft = {index.x, index.y + 1};
            if (index.x != (layout.getMICols(index.y + 1) - 1)) {
                indices.downright = {index.x + 1, index.y + 1};
            }
        }
    }

    return indices;
}

} // namespace tlct::cvt::tspc::_hp
