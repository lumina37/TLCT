#pragma once

#include "concepts.hpp"
#include "raytrix/multiview.hpp"
#include "raytrix/patchsize.hpp"
#include "raytrix/state.hpp"

namespace tlct::cvt::raytrix {

namespace _ = _cvt::raytrix;

using _::FarNeighbors;
using _::NearNeighbors;
using _::State;

static_assert(tlct::cvt::concepts::CNeighbors<NearNeighbors>);

} // namespace tlct::cvt::raytrix
