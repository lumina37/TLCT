#pragma once

#include <source_location>
#include <string>

namespace tlct {

enum class ErrCode {
    InvalidParam,
    FileSysError,
    OutOfMemory,
};

class Error {
public:
    ErrCode code;
    std::string msg;

    Error(ErrCode code, const std::source_location& srcLoc = std::source_location::current());
    Error(ErrCode code, const std::string& msg, const std::source_location& srcLoc = std::source_location::current());
    Error(const Error& rhs) = default;
    Error(Error&& rhs) noexcept = default;
};

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/error.cpp"
#endif
