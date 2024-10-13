#pragma once

#include "concepts.hpp"
#include "tspc/multiview.hpp"
#include "tspc/patchsize.hpp"
#include "tspc/state.hpp"

namespace tlct::cvt::tspc {

namespace _ = _cvt::tspc;

using _::Neighbors;
using _::State;

static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::cvt::tspc
