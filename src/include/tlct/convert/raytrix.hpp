#pragma once

#include "tlct/convert/concepts.hpp"
#include "tlct/convert/raytrix/multiview.hpp"
#include "tlct/convert/raytrix/patchsize.hpp"
#include "tlct/convert/raytrix/state.hpp"

namespace tlct::cvt::raytrix {

namespace _ = _cvt::raytrix;

using _::State;
static_assert(concepts::CState<State>);

using _::NearNeighbors;
static_assert(tlct::cvt::concepts::CNeighbors<NearNeighbors>);

using _::FarNeighbors;
static_assert(tlct::cvt::concepts::CNeighbors<FarNeighbors>);

} // namespace tlct::cvt::raytrix
