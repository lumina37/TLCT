#pragma once

#include "tlct/config/layout.hpp"
#include "tlct/convert.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace tspc {

using Layout = tlct::cfg::CornersLayout;
using StateYuv420 = tlct::cvt::State_<Layout, tlct::io::Yuv420Frame, true, false>;

} // namespace tspc

namespace _cvt {

template class State_<tlct::cfg::CornersLayout, tlct::io::Yuv420Frame, true, false>;

}

} // namespace tlct
