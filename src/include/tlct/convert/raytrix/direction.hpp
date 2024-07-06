#pragma once

#include <array>

namespace tlct::cvt::raytrix::_hp {

enum class Direction {
    UPLEFT,
    UP,
    UPRIGHT,
    DOWNRIGHT,
    DOWN,
    DOWNLEFT,
};

constexpr int DIRECTION_NUM = 6;
constexpr std::array<Direction, DIRECTION_NUM> DIRECTIONS{Direction::UPLEFT,    Direction::UP,   Direction::UPRIGHT,
                                                          Direction::DOWNRIGHT, Direction::DOWN, Direction::DOWNLEFT};

[[nodiscard]] constexpr Direction opposite(const Direction direction)
{
    switch (direction) {
    case Direction::UP:
        return Direction::DOWN;
    case Direction::DOWN:
        return Direction::UP;
    case Direction::UPLEFT:
        return Direction::DOWNRIGHT;
    case Direction::UPRIGHT:
        return Direction::DOWNLEFT;
    case Direction::DOWNLEFT:
        return Direction::UPRIGHT;
    case Direction::DOWNRIGHT:
        return Direction::UPLEFT;
    }
}

} // namespace tlct::cvt::raytrix::_hp

namespace tlct::cvt::raytrix {

using _hp::Direction;
using _hp::DIRECTION_NUM;
using _hp::DIRECTIONS;
using _hp::opposite;

} // namespace tlct::cvt::raytrix
