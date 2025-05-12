#include <source_location>
#include <string>
#include <utility>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/error.hpp"
#endif

namespace tlct {

Error::Error(const ErrCode code, const std::source_location& source) : code(code), source(source) {}

Error::Error(const ErrCode code, const std::string& msg, const std::source_location& source)
    : code(code), source(source), msg(msg) {}

Error::Error(const ErrCode code, std::string&& msg, const std::source_location& source)
    : code(code), source(source), msg(std::move(msg)) {}

}  // namespace tlct
