#pragma once

namespace tlct::_hp {

#ifdef _WIN32

#    include <string>

#    pragma push_macro("min")
#    pragma push_macro("max")
#    define WIN32_LEAN_AND_MEAN
#    include <Windows.h>
#    pragma pop_macro("min")
#    pragma pop_macro("max")

std::wstring utf8ToWstring(const std::string_view& utf8StrView) {
    int wcharSize = MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), nullptr, 0);
    if (wcharSize == 0) [[unlikely]] {
        return {};
    }
    std::wstring wstr(wcharSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), wstr.data(), wcharSize);
    return wstr;
}

std::string wstringToGBK(const std::wstring_view& wstrView) {
    int gbkSize = WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), nullptr, 0, nullptr, nullptr);
    if (gbkSize == 0) [[unlikely]] {
        return {};
    }
    std::string gbkStr(gbkSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), gbkStr.data(), gbkSize, nullptr, nullptr);
    return gbkStr;
}

std::string cconv(const std::string_view& utf8StrView) {
    std::wstring wstr = utf8ToWstring(utf8StrView);
    return wstringToGBK(wstr);
}

#endif

}  // namespace tlct::_hp
