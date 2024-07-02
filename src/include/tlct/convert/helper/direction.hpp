#pragma once

namespace tlct::cvt::_hp {

constexpr int DIRECTION_NUM = 6;

enum class Direction {
    LEFT,
    RIGHT,
    UPLEFT,
    UPRIGHT,
    DOWNLEFT,
    DOWNRIGHT,
};

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

} // namespace tlct::cvt::_hp

namespace tlct::cvt {

using _hp::Direction;
using _hp::opposite;

} // namespace tlct::cvt
