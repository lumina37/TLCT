#include <string>

#include "tlct/common/config.h"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.hpp"
#endif

namespace tlct {

std::string_view compileInfo{TLCT_COMPILE_INFO};
std::string_view version{TLCT_VERSION};

}  // namespace tlct
