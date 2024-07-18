#pragma once

#include "tlct/config/tspc.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper/neighbors.hpp"

namespace tlct::_cvt::tspc {

namespace tcfg = tlct::cfg::tspc;

using Neighbors = Neighbors_<tcfg::Layout>;

static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::_cvt::tspc
