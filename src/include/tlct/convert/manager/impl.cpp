#include "tlct/convert/concepts/manager.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/manager/impl.hpp"
#endif

namespace tlct::_cvt {

static_assert(concepts::CManager<TSPCMeth1Manager>);
template class Manager_<TSPCCensusManagerTraits>;

static_assert(concepts::CManager<RaytrixMeth1Manager>);
template class Manager_<RaytrixCensusManagerTraits>;

}  // namespace tlct::_cvt
