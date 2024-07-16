#pragma once

#include "tlct/config/raytrix/layout.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/helper/neighbors.hpp"

namespace tlct::cvt::raytrix::_hp {

namespace tcfg = tlct::cfg::raytrix;
namespace tcvthp = tlct::cvt::_hp;

using Neighbors = tcvthp::Neighbors_<tcfg::Layout>;

static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::cvt::raytrix::_hp