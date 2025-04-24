#include <filesystem>
#include <format>
#include <source_location>
#include <string>

#include "tlct/common/info.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/common/error.hpp"
#endif

namespace tlct {

namespace fs = std::filesystem;

Error::Error(const ErrCode code, const std::source_location& srcLoc) : code(code) {
    const fs::path absFilePath{srcLoc.file_name()};
    const fs::path relFilePath = fs::relative(absFilePath, includeBase);
    this->msg = std::format("{}:{}", relFilePath.string(), srcLoc.line());
}

Error::Error(const ErrCode code, const std::string& msg, const std::source_location& srcLoc) : code(code) {
    const fs::path absFilePath{srcLoc.file_name()};
    const fs::path relFilePath = fs::relative(absFilePath, includeBase);
    this->msg = std::format("{}:{} {}", relFilePath.string(), srcLoc.line(), msg);
}

}  // namespace tlct
