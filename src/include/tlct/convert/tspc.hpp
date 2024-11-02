#pragma once

#include "tlct/convert/concepts.hpp"
#include "tlct/convert/tspc/multiview.hpp"
#include "tlct/convert/tspc/patchsize.hpp"
#include "tlct/convert/tspc/state.hpp"

namespace tlct::cvt::tspc {

namespace _ = _cvt::tspc;

using _::Neighbors;
using _::State;

static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::cvt::tspc
