#pragma once

#include <source_location>
#include <string>

#include "tlct/common/defines.h"

namespace tlct {

enum class ErrCode {
    InvalidParam,
    FileSysError,
    OutOfMemory,
};

class Error {
public:
    ErrCode code;
    std::source_location source;
    std::string msg;

    TLCT_API explicit Error(ErrCode code, const std::source_location& source = std::source_location::current());
    TLCT_API Error(ErrCode code, const std::string& msg, const std::source_location& source = std::source_location::current());
    TLCT_API Error(ErrCode code, std::string&& msg, const std::source_location& source = std::source_location::current());
    Error(const Error& rhs) = default;
    Error(Error&& rhs) noexcept = default;
};

}  // namespace tlct

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/error.cpp"
#endif
