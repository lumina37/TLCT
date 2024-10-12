#pragma once

#include <array>

namespace tlct::_cvt {

enum class Direction {
    LEFT,
    UPLEFT,
    UPRIGHT,
    RIGHT,
    DOWNRIGHT,
    DOWNLEFT,
    COUNT,
};

constexpr int DIRECTION_NUM = (int)Direction::COUNT;
constexpr std::array<Direction, DIRECTION_NUM> DIRECTIONS{Direction::LEFT,  Direction::UPLEFT,    Direction::UPRIGHT,
                                                          Direction::RIGHT, Direction::DOWNRIGHT, Direction::DOWNLEFT};

} // namespace tlct::_cvt
