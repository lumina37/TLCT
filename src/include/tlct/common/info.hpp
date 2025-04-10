#pragma once

#include <string>

namespace tlct {

extern std::string_view compileInfo;
extern std::string_view version;

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.cpp"
#endif

