#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>

namespace tlct::_hp {

template <std::floating_point Tv>
[[nodiscard]] static constexpr inline int iround(Tv v) noexcept {
    return int(v + 0.5);
}

template <size_t to, std::integral Tv>
    requires(to % 2 == 0)
[[nodiscard]] static constexpr inline Tv roundTo(Tv v) noexcept {
    constexpr Tv halfTo = to >> 1;
    return (v + halfTo) & ((~to) + 1);
}

template <std::unsigned_integral Tv>
[[nodiscard]] static constexpr inline bool isPowOf2(const Tv v) noexcept {
    return (v & (v - 1)) == 0;
}

template <size_t base, std::integral T>
    requires(isPowOf2(base))
[[nodiscard]] static constexpr inline bool isMulOf(T v) noexcept {
    return (v & (base - 1)) == 0;
}

template <std::integral T>
[[nodiscard]] static constexpr inline bool isMulOf(T v, size_t shift) noexcept {
    return (v & ((1 << shift) - 1)) == 0;
}

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline Tv alignUp(Tv v) noexcept {
    return (v + (to - 1)) & ((~to) + 1);
}

template <size_t to, std::integral Tv>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline Tv alignDown(Tv v) noexcept {
    return v & ((~to) + 1);
}

// true -> +1, false -> -1
[[nodiscard]] static constexpr inline int sgn(bool v) noexcept { return ((int)v) * 2 - 1; }

template <std::floating_point Tv>
[[nodiscard]] static constexpr inline Tv sigmoid(Tv v) noexcept {
    return (Tv)1 / ((Tv)1 + exp(-v));
}

template <std::totally_ordered Tv>
[[nodiscard]] static constexpr inline Tv clip(Tv v, Tv lo, Tv hi) noexcept {
    return std::min(std::max(v, lo), hi);
}

}  // namespace tlct::_hp
