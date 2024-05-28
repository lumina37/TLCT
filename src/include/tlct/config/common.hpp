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

class CommonParamConfig
{
public:
    TLCT_API CommonParamConfig() : cfg_map_() {};
    TLCT_API CommonParamConfig& operator=(const CommonParamConfig& common_cfg) = default;
    TLCT_API CommonParamConfig(const CommonParamConfig& common_cfg) = default;
    TLCT_API CommonParamConfig& operator=(CommonParamConfig&& common_cfg) noexcept = default;
    TLCT_API CommonParamConfig(CommonParamConfig&& common_cfg) noexcept = default;
    TLCT_API explicit CommonParamConfig(ConfigMap cfg_map) : cfg_map_(std::move(cfg_map)){};

    [[nodiscard]] TLCT_API static inline CommonParamConfig fromPath(const char* path);

    [[nodiscard]] TLCT_API bool isEmpty() const noexcept;
    [[nodiscard]] TLCT_API int getCameraType() const noexcept;
    [[nodiscard]] TLCT_API const ConfigMap& getConfigMap() const noexcept;

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

inline int CommonParamConfig::getCameraType() const noexcept
{
    const auto it = cfg_map_.find("camType");
    if (it == cfg_map_.end()) {
        return 0;
    }
    const std::string& val = it->second;
    const int ival = std::stoi(val);
    return ival;
}

inline const ConfigMap& CommonParamConfig::getConfigMap() const noexcept { return cfg_map_; }

} // namespace tlct::cfg
