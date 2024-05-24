#pragma once

namespace tlct::_hp {

constexpr int iround(double v) { return int(v + 0.5); }

constexpr int align_to_2(int v) { return (v + 1) / 2 * 2; }

} // namespace tlct::_hp