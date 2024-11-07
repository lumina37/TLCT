#pragma once

#include "tlct/convert/concepts.hpp"
#include "tlct/convert/tspc/multiview.hpp"
#include "tlct/convert/tspc/patchsize.hpp"
#include "tlct/convert/tspc/state.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::tspc {

using StateYuv420 = State_<io::Yuv420Frame>;
static_assert(concepts::CState<StateYuv420>);
template class State_<io::Yuv420Frame>;

} // namespace tlct::_cvt::tspc

namespace tlct::cvt::tspc {

namespace _ = _cvt::tspc;

using _::State_;
using _::StateYuv420;

using _::Neighbors;
static_assert(tlct::cvt::concepts::CNeighbors<Neighbors>);

} // namespace tlct::cvt::tspc
