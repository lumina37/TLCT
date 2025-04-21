#pragma once

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
    std::string msg = "";
};

}  // namespace tlct
