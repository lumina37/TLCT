#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/config/tspc.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper/neighbors.hpp"

namespace tlct::_cvt::tspc {

using Neighbors = NearNeighbors_<tlct::cfg::tspc::Layout>;
static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::_cvt::tspc
