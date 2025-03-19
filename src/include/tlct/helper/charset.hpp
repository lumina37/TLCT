#pragma once

#ifdef _WIN32
#    include <string>
#    include <string_view>
#endif

#ifdef _WIN32

namespace tlct::_hp {

[[nodiscard]] std::wstring utf8ToWstring(std::string_view utf8StrView);

[[nodiscard]] std::string wstringToGBK(std::wstring_view wstrView);

[[nodiscard]] std::string cconv(std::string_view utf8StrView);

}  // namespace tlct::_hp

#endif

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/charset.cpp"
#endif
