#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/config/raytrix.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper/neighbors.hpp"

namespace tlct::_cvt::raytrix {

using NearNeighbors = NearNeighbors_<tlct::cfg::raytrix::Layout>;
static_assert(tlct::cvt::concepts::CNeighbors<NearNeighbors>);

using FarNeighbors = FarNeighbors_<tlct::cfg::raytrix::Layout>;
static_assert(tlct::cvt::concepts::CNeighbors<FarNeighbors>);

} // namespace tlct::_cvt::raytrix
