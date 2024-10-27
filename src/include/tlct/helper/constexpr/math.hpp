#pragma once

#include <cmath>
#include <concepts>
#include <cstddef>
#include <numeric>

namespace tlct::_hp {

template <std::floating_point Tv>
[[nodiscard]] static constexpr inline int iround(Tv v) noexcept
{
    return int(v + 0.5);
}

template <size_t to, std::integral Tv>
    requires(to % 2 == 0)
[[nodiscard]] static constexpr inline Tv roundTo(Tv v) noexcept
{
    constexpr Tv half_to = to >> 1;
    return (v + half_to) / to * to;
}

template <std::unsigned_integral Tv>
[[nodiscard]] static constexpr inline bool isPowOf2(const Tv v) noexcept
{
    return (v & (v - 1)) == 0;
}

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline Tv alignUp(Tv v) noexcept
{
    return (v + (to - 1)) & ((~to) + 1);
};

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline Tv alignDown(Tv v) noexcept
{
    return v & ((~to) + 1);
};

// true -> +1, false -> -1
[[nodiscard]] static constexpr inline int sgn(bool v) noexcept { return ((int)v) * 2 - 1; }

template <std::floating_point Tv>
[[nodiscard]] static constexpr inline Tv sigmoid(Tv v)
{
    return 1. / (1. + exp(-v));
}

} // namespace tlct::_hp
