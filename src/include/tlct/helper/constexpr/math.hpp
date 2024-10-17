#pragma once

#include <concepts>
#include <cstddef>

namespace tlct::_hp {

template <std::floating_point T>
[[nodiscard]] static constexpr inline int iround(T v) noexcept
{
    return int(v + 0.5);
}

template <typename Tv>
concept CMultiplicable = requires {
    { std::declval<Tv>() * std::declval<Tv>() } -> std::same_as<Tv>;
} && requires {
    { std::declval<Tv>() * (Tv)0 == (Tv)0 };
} && requires(Tv v) {
    { v*(Tv)1 == v };
};

template <CMultiplicable Tv, Tv Base, size_t Expo>
struct Pow {
    static constexpr Tv value = Base * Pow<Tv, Base, Expo - 1>::value;
};

template <CMultiplicable Tv, Tv Base>
struct Pow<Tv, Base, 0> {
    static constexpr auto value = (Tv)1;
};

template <CMultiplicable Tv, size_t Expo, size_t... Seq>
[[nodiscard]] constexpr inline auto _make_pow_array(std::index_sequence<Seq...>) noexcept
{
    return std::array<Tv, sizeof...(Seq)>{Pow<Tv, (Tv)Seq, Expo>::value...};
}

template <CMultiplicable Tv, size_t N, size_t Expo>
[[nodiscard]] constexpr inline auto make_pow_array() noexcept
{
    return _make_pow_array<Tv, Expo>(std::make_index_sequence<N>{});
}

template <size_t to, std::integral T>
    requires(to % 2 == 0)
[[nodiscard]] static constexpr inline T roundTo(T v) noexcept
{
    constexpr T half_to = to >> 1;
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

} // namespace tlct::_hp
