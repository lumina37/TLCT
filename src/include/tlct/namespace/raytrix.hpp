#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"

namespace tlct {

namespace raytrix {

using Arrange = cfg::OffsetArrange;
using ManagerYuv420 = cvt::Manager_<Arrange>;

}  // namespace raytrix

}  // namespace tlct
