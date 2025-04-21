#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"

namespace tlct {

namespace tspc {

using Arrange = cfg::CornersArrange;
using ManagerYuv420 = cvt::Manager_<Arrange, true, false>;

static_assert(cvt::concepts::CManager<ManagerYuv420>);

}  // namespace tspc

namespace _cvt {

template class Manager_<cfg::CornersArrange, true, false>;

}

}  // namespace tlct
