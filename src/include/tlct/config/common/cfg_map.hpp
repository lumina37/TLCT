#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "enums.hpp"
#include "tlct/common/defines.h"

namespace tlct::_cfg {

class ConfigMap
{
public:
    // Typename alias
    using TMap = std::map<std::string, std::string>;

    // Constructor
    TLCT_API inline ConfigMap() : map_(){};
    TLCT_API inline ConfigMap& operator=(const ConfigMap& rhs) = default;
    TLCT_API inline ConfigMap(const ConfigMap& rhs) = default;
    TLCT_API inline ConfigMap& operator=(ConfigMap&& rhs) noexcept = default;
    TLCT_API inline ConfigMap(ConfigMap&& rhs) noexcept = default;
    TLCT_API explicit inline ConfigMap(TMap cfg_map) : map_(std::move(cfg_map)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ConfigMap fromFstream(std::ifstream&& ifs) noexcept;
    [[nodiscard]] TLCT_API static inline ConfigMap fromPath(std::string_view path);

    // Const methods
    [[nodiscard]] TLCT_API inline bool isEmpty() const noexcept;
    [[nodiscard]] TLCT_API inline int getPipelineType() const noexcept;
    [[nodiscard]] TLCT_API inline const TMap& getMap() const noexcept;

    template <typename Tv>
    [[nodiscard]] TLCT_API inline Tv get(const std::string& key) const;
    template <typename Tv>
    [[nodiscard]] TLCT_API inline Tv get(const std::string& key, const Tv& default_val) const;

private:
    TMap map_;
};

ConfigMap ConfigMap::fromFstream(std::ifstream&& ifs) noexcept
{
    auto has_delim = [](const char c) { return c == '\t' || c == ' '; };

    std::map<std::string, std::string> cfg_map;
    std::string row;
    while (std::getline(ifs, row)) {
        if (row.empty() || row.starts_with('=')) {
            break;
        }

        std::string::iterator key_end = row.begin();
        for (; key_end != row.end(); key_end++) {
            if (has_delim(*key_end)) {
                break;
            }
        }

        if (key_end == row.end()) {
            // row without KV
            continue;
        }

        std::string::iterator value_start = key_end;
        for (; value_start != row.end(); value_start++) {
            if (!has_delim(*value_start)) {
                break;
            }
        }

        const std::string_view key{row.begin(), key_end};
        const std::string_view value{value_start, row.end()};
        cfg_map.emplace(key, value);
    }

    return ConfigMap(cfg_map);
}

ConfigMap ConfigMap::fromPath(std::string_view path)
{
    std::ifstream ifs(path.data());
    if (!ifs) {
        std::cerr << "Failed to load `ConfigMap` from `" << path << "`!" << std::endl;
        return {};
    }

    return fromFstream(std::move(ifs));
}

bool ConfigMap::isEmpty() const noexcept { return map_.empty(); }

int ConfigMap::getPipelineType() const noexcept { return get("pipeline", (int)PipelineType::RLC); }

template <typename Tv>
inline Tv stox(const std::string& str);

template <>
inline int stox(const std::string& str)
{
    return std::stoi(str);
}

template <>
inline float stox(const std::string& str)
{
    return std::stof(str);
}

template <>
inline double stox(const std::string& str)
{
    return std::stod(str);
}

template <typename Tv>
Tv ConfigMap::get(const std::string& key, const Tv& default_val) const
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    const std::string& val = it->second;
    const float nval = stox<Tv>(val);
    return nval;
};

template <typename Tv>
Tv ConfigMap::get(const std::string& key) const
{
    return stox<Tv>(map_.at(key));
};

template <>
std::string ConfigMap::get(const std::string& key, const std::string& default_val) const
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    return it->second;
};

template <>
std::string ConfigMap::get(const std::string& key) const
{
    return map_.at(key);
};

const ConfigMap::TMap& ConfigMap::getMap() const noexcept { return map_; }

} // namespace tlct::_cfg
