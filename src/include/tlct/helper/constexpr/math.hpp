#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <numeric>

namespace tlct::_hp {

template <std::floating_point Tv>
[[nodiscard]] static inline constexpr int iround(Tv v) noexcept
{
    return int(v + 0.5);
}

template <size_t to, std::integral Tv>
    requires(to % 2 == 0)
[[nodiscard]] static inline constexpr Tv roundTo(Tv v) noexcept
{
    constexpr Tv half_to = to >> 1;
    return (v + half_to) / to * to;
}

template <std::unsigned_integral Tv>
[[nodiscard]] static inline constexpr bool isPowOf2(const Tv v) noexcept
{
    return (v & (v - 1)) == 0;
}

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static inline constexpr Tv alignUp(Tv v) noexcept
{
    return (v + (to - 1)) & ((~to) + 1);
};

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static inline constexpr Tv alignDown(Tv v) noexcept
{
    return v & ((~to) + 1);
};

// true -> +1, false -> -1
[[nodiscard]] static inline constexpr int sgn(bool v) noexcept { return ((int)v) * 2 - 1; }

template <std::floating_point Tv>
[[nodiscard]] static inline constexpr Tv sigmoid(Tv v)
{
    return 1. / (1. + exp(-v));
}

template <std::totally_ordered Tv>
[[nodiscard]] static constexpr inline Tv clip(Tv v, Tv lo, Tv hi) noexcept
{
    return std::min(std::max(v, lo), hi);
}

} // namespace tlct::_hp
