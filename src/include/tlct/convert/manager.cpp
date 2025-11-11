#pragma once

#include "tlct/config/arrange.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/manager.hpp"
#endif

namespace tlct::_cvt {

template class Manager_<cfg::CornersArrange>;
template class Manager_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
