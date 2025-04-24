#pragma once

#include <filesystem>
#include <string>

namespace tlct {

namespace fs = std::filesystem;

extern const std::string_view compileInfo;
extern const std::string_view version;
extern const fs::path includeBase;

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.cpp"
#endif
