#pragma once

#include "tlct/convert/concepts/bridge.hpp"
#include "tlct/convert/concepts/manager.hpp"
#include "tlct/convert/concepts/multiview.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/concepts/patchsize.hpp"

namespace tlct::cvt::concepts {

namespace _ = _cvt::concepts;

using _::CBridge;
using _::CManager;
using _::CManagerTraits;
using _::CMvImpl;
using _::CNeighbors;
using _::CPatchMergeBridge;
using _::CPsizeImpl;

}  // namespace tlct::cvt::concepts
