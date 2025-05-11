#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"

namespace tlct {

namespace raytrix {

using Arrange = cfg::OffsetArrange;
using ManagerYuv420 = cvt::Manager_<Arrange>;

static_assert(cvt::concepts::CManager<ManagerYuv420>);

}  // namespace raytrix

namespace _cvt {

template class Manager_<cfg::OffsetArrange>;

}

}  // namespace tlct
