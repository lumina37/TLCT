#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"

namespace tlct {

namespace tspc {

using Arrange = cfg::CornersArrange;
using ManagerYuv420 = cvt::Manager_<Arrange>;

}  // namespace tspc

}  // namespace tlct
