#pragma once

#include "tlct/convert/concepts.hpp"
#include "tlct/convert/raytrix/multiview.hpp"
#include "tlct/convert/raytrix/patchsize.hpp"
#include "tlct/convert/raytrix/state.hpp"

namespace tlct::cvt::raytrix {

namespace _ = _cvt::raytrix;

using _::FarNeighbors;
using _::NearNeighbors;
using _::State;

static_assert(tlct::cvt::concepts::CNeighbors<NearNeighbors>);

} // namespace tlct::cvt::raytrix
