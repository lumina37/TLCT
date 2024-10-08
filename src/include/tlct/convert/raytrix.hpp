#pragma once

#include "concepts.hpp"
#include "patchsize/neighbors.hpp"
#include "raytrix/multiview.hpp"
#include "raytrix/state.hpp"

namespace tlct {

namespace cvt::raytrix {

namespace _ = _cvt::raytrix;

using _::State;

using Neighbors = _cvt::Neighbors_<tlct::cfg::raytrix::Layout>;
static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace cvt::raytrix

namespace _cvt {

template class Neighbors_<tlct::cfg::raytrix::Layout>;

}

} // namespace tlct
