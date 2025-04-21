#pragma once

#include <string>

namespace tlct {

enum class ErrCode {
    InvalidParam,
    FileSysError,
};

class Error {
public:
    ErrCode code;
    std::string msg = "";
};

}  // namespace tlct
