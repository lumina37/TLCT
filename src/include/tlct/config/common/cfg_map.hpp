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
    [[nodiscard]] TLCT_API static inline ConfigMap fromPath(const std::string_view& path);

    // Const methods
    [[nodiscard]] TLCT_API inline bool isEmpty() const noexcept;
    [[nodiscard]] TLCT_API inline int getPipelineType() const noexcept;
    [[nodiscard]] TLCT_API inline const TMap& getMap() const noexcept;

    template <typename Tv>
    [[nodiscard]] TLCT_API inline Tv get(const std::string& key) const;
    template <typename Tv>
    [[nodiscard]] TLCT_API inline Tv get(const std::string& key, const Tv& default_val) const noexcept;

private:
    TMap map_;
};

ConfigMap ConfigMap::fromPath(const std::string_view& path)
{
    std::ifstream fs(path.data());
    if (!fs) {
        std::cerr << "Failed to load `" << typeid(ConfigMap).name() << "` from `" << path << "`!" << std::endl;
        return {};
    }

    std::map<std::string, std::string> cfg_map;
    std::string row;
    while (std::getline(fs, row)) {
        if (row.empty() || row.starts_with('=')) {
            break;
        }
        std::istringstream srow(row);
        std::string key, value;
        srow >> key;
        srow >> value;
        cfg_map[key] = value;
    }

    return ConfigMap(cfg_map);
}

bool ConfigMap::isEmpty() const noexcept { return map_.empty(); }

int ConfigMap::getPipelineType() const noexcept
{
    const auto it = map_.find("pipeline");
    if (it == map_.end()) {
        return (int)PipelineType::RLC;
    }
    const std::string& val = it->second;
    const int ival = std::stoi(val);
    return ival;
}

template <>
int ConfigMap::get(const std::string& key, const int& default_val) const noexcept
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    const std::string& val = it->second;
    const int nval = std::stoi(val);
    return nval;
};

template <>
int ConfigMap::get(const std::string& key) const
{
    return std::stoi(map_.at(key));
};

template <>
double ConfigMap::get(const std::string& key, const double& default_val) const noexcept
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    const std::string& val = it->second;
    const double nval = std::stod(val);
    return nval;
};

template <>
double ConfigMap::get(const std::string& key) const
{
    return std::stod(map_.at(key));
};

template <>
float ConfigMap::get(const std::string& key, const float& default_val) const noexcept
{
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return default_val;
    }
    const std::string& val = it->second;
    const float nval = std::stof(val);
    return nval;
};

template <>
float ConfigMap::get(const std::string& key) const
{
    return std::stof(map_.at(key));
};

template <>
std::string ConfigMap::get(const std::string& key, const std::string& default_val) const noexcept
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
