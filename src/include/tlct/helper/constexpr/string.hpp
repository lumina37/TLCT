#pragma once

#include <algorithm>

#include "tlct/helper/std.hpp"

namespace tlct::_hp {

template <size_t N>
class cestring {
public:
    consteval cestring(const char (&str)[N]) noexcept { std::copy(str, str + N, string); }

    char string[N]{};
};

}  // namespace tlct::_hp
