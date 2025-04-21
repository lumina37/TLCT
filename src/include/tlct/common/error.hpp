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
    std::string msg;

    Error(ErrCode code) : code(code) {}
    Error(ErrCode code, const std::string& msg) : code(code), msg(msg) {}
    Error(ErrCode code, std::string&& msg) : code(code), msg(std::move(msg)) {}
    Error& operator=(const Error& rhs) = default;
    Error(const Error& rhs) = default;
    Error& operator=(Error&& rhs) noexcept = default;
    Error(Error&& rhs) noexcept = default;
};

}  // namespace tlct
