#pragma once

#include <concepts>

namespace tlct::_hp {

template <typename T>
    requires std::is_floating_point_v<T>
static constexpr inline int iround(T v)
{
    return int(v + 0.5);
}

template <size_t to, typename T>
    requires std::is_integral_v<T> && (to % 2 == 0)
static constexpr inline T round_to(T v)
{
    constexpr T half_to = to >> 1;
    return (v + half_to) / to * to;
}

template <typename Tv>
    requires std::is_unsigned_v<Tv>
static constexpr inline bool is_pow_of_2(const Tv v)
{
    return (v & (v - 1)) == 0;
}

template <size_t to, typename T>
    requires std::is_integral_v<T> && (is_pow_of_2(to))
static constexpr inline T align_up(T v)
{
    return (v + (to - 1)) & ((~to) + 1);
};

template <size_t to, typename T>
    requires std::is_integral_v<T> && (is_pow_of_2(to))
static constexpr inline T align_down(T v)
{
    return v & ((~to) + 1);
};

// true -> +1, false -> -1
static constexpr inline int sgn(bool v) { return ((int)v) * 2 - 1; }

} // namespace tlct::_hp
