#pragma once

#include "tlct/config/arrange.hpp"
#include "tlct/convert.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace tspc {

using Arrange = tlct::cfg::CornersArrange;
using ManagerYuv420 = tlct::cvt::Manager_<Arrange, tlct::io::Yuv420Frame, true, false>;

} // namespace tspc

namespace _cvt {

template class Manager_<tlct::cfg::CornersArrange, tlct::io::Yuv420Frame, true, false>;

}

} // namespace tlct
