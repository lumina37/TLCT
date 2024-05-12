#pragma once

#include <fstream>
#include <map>
#include <ranges>
#include <sstream>
#include <string>

#include "tlct/common/defines.h"

namespace tlct::cfg {

namespace rgs = std::ranges;

using ConfigMap = std::map<std::string, std::string>;

class TLCT_API CommonParamConfig
{
public:
    CommonParamConfig() : cfg_map_(){};
    explicit CommonParamConfig(const std::map<std::string, std::string>& cfg_map) : cfg_map_(cfg_map){};

    static CommonParamConfig fromPath(const char* path);

    [[nodiscard]] bool isEmpty() const noexcept;
    [[nodiscard]] bool isTSPC() const noexcept;
    [[nodiscard]] const ConfigMap& getConfigMap() const noexcept;

private:
    ConfigMap cfg_map_;
};

inline CommonParamConfig CommonParamConfig::fromPath(const char* path)
{
    std::ifstream fs(path);
    if (!fs) {
        return {};
    }

    std::map<std::string, std::string> cfg_map;
    std::string row;
    while (std::getline(fs, row)) {
        std::istringstream srow(row);
        std::string key, value;
        srow >> key;
        srow >> value;
        cfg_map[key] = value;
    }

    return CommonParamConfig(cfg_map);
}

inline bool CommonParamConfig::isEmpty() const noexcept { return cfg_map_.empty(); }

inline bool CommonParamConfig::isTSPC() const noexcept
{
    const auto it = cfg_map_.find("isTSPC");
    if (it == cfg_map_.end()) {
        return false;
    }
    const std::string& val = it->second;
    const int ival = std::stoi(val);
    return ival != 0;
}

inline const ConfigMap& CommonParamConfig::getConfigMap() const noexcept { return cfg_map_; }

} // namespace tlct::cfg
