#pragma once

#include "tlct/convert/concepts.hpp"
#include "tlct/convert/raytrix/multiview.hpp"
#include "tlct/convert/raytrix/patchsize.hpp"
#include "tlct/convert/raytrix/state.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::raytrix {

using StateYuv420 = State_<io::Yuv420Frame>;
static_assert(concepts::CState<StateYuv420>);
template class State_<io::Yuv420Frame>;

} // namespace tlct::_cvt::raytrix

namespace tlct::cvt::raytrix {

namespace _ = _cvt::raytrix;

using _::State_;
using _::StateYuv420;

using _::NearNeighbors;
static_assert(tlct::cvt::concepts::CNeighbors<NearNeighbors>);

using _::FarNeighbors;
static_assert(tlct::cvt::concepts::CNeighbors<FarNeighbors>);

} // namespace tlct::cvt::raytrix
