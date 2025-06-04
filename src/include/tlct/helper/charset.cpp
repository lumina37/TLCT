#ifdef _WIN32

#    include <expected>
#    include <string>

#    include "tlct/helper/error.hpp"

#    ifndef _TLCT_LIB_HEADER_ONLY
#        include "tlct/helper/charset.hpp"
#    endif

namespace tlct::_hp {

#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <Windows.h>

std::expected<std::wstring, Error> utf8ToWstring(const std::string_view utf8StrView) noexcept {
    int wcharSize = MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), nullptr, 0);
    if (wcharSize == 0) [[unlikely]] {
        return {};
    }
    std::wstring wstr(wcharSize, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8StrView.data(), (int)utf8StrView.size(), wstr.data(), wcharSize);
    return wstr;
}

std::expected<std::string, Error> wstringToLocal(const std::wstring_view wstrView) noexcept {
    int localSize = WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), nullptr, 0, nullptr, nullptr);
    if (localSize == 0) [[unlikely]] {
        return {};
    }
    std::string localStr(localSize, 0);
    WideCharToMultiByte(CP_ACP, 0, wstrView.data(), (int)wstrView.size(), localStr.data(), localSize, nullptr, nullptr);
    return localStr;
}

std::expected<std::string, Error> cconv(const std::string_view utf8StrView) noexcept {
    auto wstrRes = utf8ToWstring(utf8StrView);
    if (!wstrRes) return std::unexpected{std::move(wstrRes.error())};
    return wstringToLocal(wstrRes.value());
}

}  // namespace tlct::_hp

#endif
