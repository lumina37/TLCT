#pragma once

namespace tlct::cvt {

enum Direction {
    LEFT = 1 << 0,
    RIGHT = 1 << 1,
    UPLEFT = 1 << 2,
    UPRIGHT = 1 << 3,
    DOWNLEFT = 1 << 4,
    DOWNRIGHT = 1 << 5,
};

constexpr int NEIGHBOR_NUM = 6;

} // namespace tlct::cvt
