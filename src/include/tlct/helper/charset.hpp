#pragma once

namespace tlct::_hp {

#ifdef _WIN32

#    include <string>

#    include <Windows.h>
#    undef min
#    undef max

std::wstring utf8_to_wstring(const std::string_view& utf8_str_view)
{
    int wchar_size = MultiByteToWideChar(CP_UTF8, 0, utf8_str_view.data(), (int)utf8_str_view.size(), nullptr, 0);
    if (wchar_size == 0) [[unlikely]] {
        return {};
    }
    std::wstring wstr(wchar_size, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8_str_view.data(), (int)utf8_str_view.size(), wstr.data(), wchar_size);
    return wstr;
}

std::string wstring_to_gbk(const std::wstring_view& wstr_view)
{
    int gbk_size =
        WideCharToMultiByte(CP_ACP, 0, wstr_view.data(), (int)wstr_view.size(), nullptr, 0, nullptr, nullptr);
    if (gbk_size == 0) [[unlikely]] {
        return {};
    }
    std::string gbk_str(gbk_size, 0);
    WideCharToMultiByte(CP_ACP, 0, wstr_view.data(), (int)wstr_view.size(), gbk_str.data(), gbk_size, nullptr, nullptr);
    return gbk_str;
}

std::string cconv(const std::string_view& utf8_str_view)
{
    std::wstring wstr = utf8_to_wstring(utf8_str_view);
    return wstring_to_gbk(wstr);
}

#endif

} // namespace tlct::_hp
