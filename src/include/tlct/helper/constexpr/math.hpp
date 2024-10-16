#pragma once

#include <concepts>
#include <cstddef>

namespace tlct::_hp {

template <std::floating_point T>
[[nodiscard]] static constexpr inline int iround(T v)
{
    return int(v + 0.5);
}

template <size_t to, std::integral T>
    requires(to % 2 == 0)
[[nodiscard]] static constexpr inline T roundTo(T v)
{
    constexpr T half_to = to >> 1;
    return (v + half_to) / to * to;
}

template <std::unsigned_integral Tv>
[[nodiscard]] static constexpr inline bool isPowOf2(const Tv v)
{
    return (v & (v - 1)) == 0;
}

template <size_t to, std::integral T>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline T alignUp(T v)
{
    return (v + (to - 1)) & ((~to) + 1);
};

template <size_t to, std::integral T>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline T alignDown(T v)
{
    return v & ((~to) + 1);
};

// true -> +1, false -> -1
[[nodiscard]] static constexpr inline int sgn(bool v) { return ((int)v) * 2 - 1; }

} // namespace tlct::_hp
