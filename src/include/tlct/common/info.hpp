#pragma once

#include <string>

namespace tlct {

extern const std::string_view compileInfo;
extern const std::string_view version;

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.cpp"
#endif

