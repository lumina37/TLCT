#pragma once

#include "tlct/convert/concepts/manager.hpp"
#include "tlct/config/arrange.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/manager.hpp"
#endif

namespace tlct::_cvt {

static_assert(concepts::CManager<Manager_<cfg::CornersArrange>>);
template class Manager_<cfg::CornersArrange>;

static_assert(concepts::CManager<Manager_<cfg::OffsetArrange>>);
template class Manager_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
