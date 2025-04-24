#include <filesystem>
#include <string>

#include "tlct/common/config.h"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/info.hpp"
#endif

namespace tlct {

namespace fs = std::filesystem;

const std::string_view compileInfo{TLCT_COMPILE_INFO};
const std::string_view version{TLCT_VERSION};
const fs::path includeBase{TLCT_INCLUDE_BASE};

}  // namespace tlct
