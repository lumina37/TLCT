#pragma once

#include <filesystem>
#include <string>

#include "tlct/common/defines.h"

namespace tlct {

namespace fs = std::filesystem;

TLCT_API extern const std::string_view compileInfo;
TLCT_API extern const std::string_view version;
TLCT_API extern const fs::path includeBase;

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.cpp"
#endif
