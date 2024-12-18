#pragma once

#include <concepts>
#include <exception>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "tlct/common/defines.h"
#include "tlct/helper.hpp"

namespace tlct::_cfg {

class ConfigMap
{
public:
    // Typename alias
    using TMap = std::map<std::string, std::string>;

    // Constructor
    TLCT_API inline ConfigMap() : map_(){};
    TLCT_API inline ConfigMap(const ConfigMap& rhs) = default;
    TLCT_API inline ConfigMap& operator=(const ConfigMap& rhs) = default;
    TLCT_API inline ConfigMap(ConfigMap&& rhs) noexcept = default;
    TLCT_API inline ConfigMap& operator=(ConfigMap&& rhs) noexcept = default;
    TLCT_API explicit inline ConfigMap(TMap&& cfg_map) noexcept : map_(std::move(cfg_map)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ConfigMap fromFstream(std::ifstream&& ifs);
    [[nodiscard]] TLCT_API static inline ConfigMap fromPath(std::string_view path);

    // Const methods
    [[nodiscard]] TLCT_API inline bool isEmpty() const noexcept;

    template <_hp::cestring key, typename Tv>
    [[nodiscard]] TLCT_API inline Tv get() const;

    template <_hp::cestring key, typename Tv>
        requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
    [[nodiscard]] TLCT_API inline Tv get_or(const Tv& default_val) const noexcept;

    template <_hp::cestring key, typename Tf>
        requires std::is_invocable_v<Tf>
    [[nodiscard]] TLCT_API inline auto get_or_else(const Tf& default_factory) const noexcept
        -> decltype(default_factory());

    template <typename Tv>
    [[nodiscard]] TLCT_API inline Tv get(const std::string& key) const;

    template <typename Tv>
        requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
    [[nodiscard]] TLCT_API inline Tv get_or(const std::string& key, const Tv& default_val) const noexcept;

    template <typename Tf>
        requires std::is_invocable_v<Tf>
    [[nodiscard]] TLCT_API inline auto get_or_else(const std::string& key, const Tf& default_factory) const noexcept
        -> decltype(default_factory());

private:
    TMap map_;
};

ConfigMap ConfigMap::fromFstream(std::ifstream&& ifs)
{
    const auto is_nul = [](const char c) { return c == '\t' || c == ' '; };

    std::map<std::string, std::string> cfg_map;
    std::string row;
    while (std::getline(ifs, row)) {
        if (row.empty()) [[unlikely]] {
            continue;
        }

        const auto delim = std::find(row.begin() + 1, row.end(), ':');
        if (delim == row.end()) [[unlikely]] {
            continue;
        }
        if (delim == std::prev(row.end())) [[unlikely]] {
            continue;
        }

        auto key_end = delim - 1;
        for (; key_end != row.begin(); key_end--) {
            if (!is_nul(*key_end)) {
                break;
            }
        }
        key_end++;

        auto value_start = delim + 1;
        const auto comment = std::find(value_start, row.end(), '#');
        for (; value_start != comment; value_start++) {
            if (!is_nul(*value_start)) {
                break;
            }
        }

#ifdef _WIN32
        const std::string& key = _hp::cconv({row.begin(), key_end});
        const std::string& value = _hp::cconv({value_start, comment});
#else
        const std::string_view& key{row.begin(), key_end};
        const std::string_view& value{value_start, row.end()};
#endif
        cfg_map.emplace(key, value);
    }

    return ConfigMap(std::move(cfg_map));
}

ConfigMap ConfigMap::fromPath(std::string_view path)
{
    std::ifstream ifs(path.data());
    if (!ifs.is_open()) [[unlikely]] {
        std::stringstream err;
        err << "Failed to load `ConfigMap` from `" << path << "`! The file may not exist.";
        throw std::runtime_error{err.str()};
    }

    return fromFstream(std::move(ifs));
}

bool ConfigMap::isEmpty() const noexcept { return map_.empty(); }

template <typename Tv>
inline Tv stox(const std::string& str)
{
    if constexpr (std::is_integral_v<Tv>) {
        return (Tv)std::stoi(str);
    } else if constexpr (std::is_floating_point_v<Tv>) {
        return (Tv)std::stod(str);
    } else {
        return str;
    }
};

template <_hp::cestring key, typename Tv>
Tv ConfigMap::get() const
{
    return this->get<Tv>(key.string);
};

template <_hp::cestring key, typename Tv>
    requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
Tv ConfigMap::get_or(const Tv& default_val) const noexcept
{
    return this->get_or<Tv>(key.string, default_val);
};

template <_hp::cestring key, typename Tf>
    requires std::is_invocable_v<Tf>
auto ConfigMap::get_or_else(const Tf& default_factory) const noexcept -> decltype(default_factory())
{
    return this->get_or_else<Tf>(key.string, default_factory);
};

template <typename Tv>
Tv ConfigMap::get(const std::string& key) const
{
    return stox<Tv>(map_.at(key));
};

template <typename Tv>
    requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
Tv ConfigMap::get_or(const std::string& key, const Tv& default_val) const noexcept
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    const std::string& val = it->second;
    const Tv nval = stox<Tv>(val);
    return nval;
};

template <typename Tf>
    requires std::is_invocable_v<Tf>
auto ConfigMap::get_or_else(const std::string& key, const Tf& default_factory) const noexcept
    -> decltype(default_factory())
{
    using Tval = decltype(default_factory());

    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_factory();
    }
    const std::string& val = it->second;
    const Tval nval = stox<Tval>(val);
    return nval;
};

} // namespace tlct::_cfg
