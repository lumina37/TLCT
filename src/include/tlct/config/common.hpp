#pragma once

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "tlct/common/defines.h"

namespace tlct::cfg {

using ConfigMap = std::map<std::string, std::string>;

class CommonParamConfig
{
public:
    // Constructor
    TLCT_API inline CommonParamConfig() : cfg_map_(){};
    TLCT_API inline CommonParamConfig& operator=(const CommonParamConfig& common_cfg) = default;
    TLCT_API inline CommonParamConfig(const CommonParamConfig& common_cfg) = default;
    TLCT_API inline CommonParamConfig& operator=(CommonParamConfig&& common_cfg) noexcept = default;
    TLCT_API inline CommonParamConfig(CommonParamConfig&& common_cfg) noexcept = default;
    TLCT_API explicit inline CommonParamConfig(ConfigMap cfg_map) : cfg_map_(std::move(cfg_map)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CommonParamConfig fromPath(const std::string_view& path);

    // Const methods
    [[nodiscard]] TLCT_API inline bool isEmpty() const noexcept;
    [[nodiscard]] TLCT_API inline int getCameraType() const noexcept;
    [[nodiscard]] TLCT_API inline const ConfigMap& getConfigMap() const noexcept;

private:
    ConfigMap cfg_map_;
};

CommonParamConfig CommonParamConfig::fromPath(const std::string_view& path)
{
    std::ifstream fs(path.data());
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

bool CommonParamConfig::isEmpty() const noexcept { return cfg_map_.empty(); }

int CommonParamConfig::getCameraType() const noexcept
{
    const auto it = cfg_map_.find("camType");
    if (it == cfg_map_.end()) {
        return 0;
    }
    const std::string& val = it->second;
    const int ival = std::stoi(val);
    return ival;
}

const ConfigMap& CommonParamConfig::getConfigMap() const noexcept { return cfg_map_; }

} // namespace tlct::cfg
