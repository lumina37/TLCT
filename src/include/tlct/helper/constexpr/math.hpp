#pragma once

#include <concepts>
#include <cstddef>
#include <numeric>

namespace tlct::_hp {

template <std::floating_point Tv>
[[nodiscard]] static constexpr inline int iround(Tv v) noexcept
{
    return int(v + 0.5);
}

template <std::totally_ordered Tv>
[[nodiscard]] static constexpr inline Tv clip(Tv v, Tv lo, Tv hi) noexcept
{
    return std::min(std::max(v, lo), hi);
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

template <size_t to, std::integral T>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline T alignUp(T v) noexcept
{
    return (v + (to - 1)) & ((~to) + 1);
};

template <size_t to, std::integral T>
    requires(isPowOf2(to))
[[nodiscard]] static constexpr inline T alignDown(T v) noexcept
{
    return v & ((~to) + 1);
};

// true -> +1, false -> -1
[[nodiscard]] static constexpr inline int sgn(bool v) noexcept { return ((int)v) * 2 - 1; }

template <typename Tv>
[[nodiscard]] static constexpr inline Tv stdvar(const std::vector<Tv>& vec)
{
    const Tv sum = std::reduce(vec.begin(), vec.end());
    const Tv avg = sum / (Tv)vec.size();

    Tv var = 0.0;
    for (const Tv elem : vec) {
        const Tv diff = elem - avg;
        var += diff * diff;
    }
    Tv stdvar = var / (Tv)vec.size();

    return stdvar;
}

} // namespace tlct::_hp
