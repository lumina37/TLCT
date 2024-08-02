#pragma once

namespace tlct::_hp {

constexpr int iround(double v) { return int(v + 0.5); }

constexpr int align_to_2(int v) { return (v + 1) / 2 * 2; }

// true -> +1, false -> -1
constexpr int sgn(bool v) { return ((int)v) * 2 - 1; }

} // namespace tlct::_hp