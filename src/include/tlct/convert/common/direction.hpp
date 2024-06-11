#pragma once

namespace tlct::cvt {

enum class Direction {
    LEFT = 1 << 0,
    RIGHT = 1 << 1,
    LEFT_UP = 1 << 2,
    RIGHT_UP = 1 << 3,
    LEFT_DOWN = 1 << 4,
    RIGHT_DOWN = 1 << 5,
};

constexpr int NEIGHBOR_NUM = 6;

} // namespace tlct::cvt
