#pragma once

#ifdef _WIN32
#    include <expected>
#    include <string>

#    include "tlct/helper/error.hpp"

namespace tlct::_hp {

[[nodiscard]] std::expected<std::string, Error> cconv(std::string_view utf8StrView) noexcept;

}  // namespace tlct::_hp

#endif

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/charset.cpp"
#endif
