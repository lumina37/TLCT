#pragma once

#include <array>

namespace tlct::cvt::tspc::_hp {

enum class Direction {
    LEFT,
    UPLEFT,
    UPRIGHT,
    RIGHT,
    DOWNRIGHT,
    DOWNLEFT,
};

constexpr int DIRECTION_NUM = 6;
constexpr std::array<Direction, DIRECTION_NUM> DIRECTIONS{Direction::LEFT,  Direction::UPLEFT,    Direction::UPRIGHT,
                                                          Direction::RIGHT, Direction::DOWNRIGHT, Direction::DOWNLEFT};

[[nodiscard]] constexpr Direction opposite(const Direction direction)
{
    switch (direction) {
    case Direction::LEFT:
        return Direction::RIGHT;
    case Direction::RIGHT:
        return Direction::LEFT;
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

} // namespace tlct::cvt::tspc::_hp

namespace tlct::cvt::tspc {

using _hp::Direction;
using _hp::DIRECTION_NUM;
using _hp::DIRECTIONS;
using _hp::opposite;

} // namespace tlct::cvt::tspc
