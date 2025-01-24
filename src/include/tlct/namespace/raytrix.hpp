#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace raytrix {

using Arrange = tlct::cfg::OffsetArrange;
using ManagerYuv420 = tlct::cvt::Manager_<Arrange, tlct::io::Yuv420Frame, false, true>;

}  // namespace raytrix

namespace _cvt {

template class Manager_<tlct::cfg::OffsetArrange, tlct::io::Yuv420Frame, false, true>;

}

}  // namespace tlct
