#pragma once

#include <string>

#include "tlct/common/defines.h"

namespace tlct {

TLCT_API extern const std::string_view compileInfo;
TLCT_API extern const std::string_view version;

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.cpp"
#endif
