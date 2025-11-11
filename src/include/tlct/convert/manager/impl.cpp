#pragma once

#include "tlct/convert/concepts/manager.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/manager/impl.hpp"
#endif

namespace tlct::_cvt {

static_assert(concepts::CManager<TSPCCensusManager>);
template class Manager_<TSPCCensusManagerTraits>;

static_assert(concepts::CManager<RaytrixCensusManager>);
template class Manager_<RaytrixCensusManagerTraits>;

}  // namespace tlct::_cvt
