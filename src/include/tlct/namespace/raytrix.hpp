#pragma once

#include "tlct/config/layout.hpp"
#include "tlct/convert.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace raytrix {

using Layout = tlct::cfg::OffsetLayout;
using ManagerYuv420 = tlct::cvt::Manager_<Layout, tlct::io::Yuv420Frame, false, true>;

} // namespace raytrix

namespace _cvt {

template class Manager_<tlct::cfg::OffsetLayout, tlct::io::Yuv420Frame, false, true>;

}

} // namespace tlct
