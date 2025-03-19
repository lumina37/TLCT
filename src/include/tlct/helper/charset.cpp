#include <string>
#include <string_view>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/helper/charset.hpp"
#endif

#ifdef _WIN32

namespace tlct::_hp {

[[nodiscard]] std::wstring utf8ToWstring(const std::string_view& utf8StrView) {
    int wcharSize = MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), nullptr, 0);
    if (wcharSize == 0) [[unlikely]] {
        return {};
    }
    std::wstring wstr(wcharSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), wstr.data(), wcharSize);
    return wstr;
}

[[nodiscard]] std::string wstringToGBK(const std::wstring_view& wstrView) {
    int gbkSize = WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), nullptr, 0, nullptr, nullptr);
    if (gbkSize == 0) [[unlikely]] {
        return {};
    }
    std::string gbkStr(gbkSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), gbkStr.data(), gbkSize, nullptr, nullptr);
    return gbkStr;
}

[[nodiscard]] std::string cconv(const std::string_view& utf8StrView) {
    std::wstring wstr = utf8ToWstring(utf8StrView);
    return wstringToGBK(wstr);
}

}  // namespace tlct::_hp

#endif
