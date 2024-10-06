#pragma once

#include "concepts.hpp"
#include "patchsize/neighbors.hpp"
#include "tspc/multiview.hpp"
#include "tspc/state.hpp"

namespace tlct::cvt::tspc {

namespace _ = _cvt::tspc;

using _::State;

using Neighbors = _cvt::Neighbors_<tlct::cfg::tspc::Layout>;
static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::cvt::tspc

namespace tlct::_cvt {

template class Neighbors_<tlct::cfg::tspc::Layout>;

}
