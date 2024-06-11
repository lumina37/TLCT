#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "tlct/common/defines.h"

namespace tlct::cfg {

enum class PipelineType { RLC, TLCT };

using ConfigMap = std::map<std::string, std::string>;

class CommonParamConfig
{
public:
    // Constructor
    TLCT_API inline CommonParamConfig() : cfg_map_(){};
    TLCT_API inline CommonParamConfig& operator=(const CommonParamConfig& rhs) = default;
    TLCT_API inline CommonParamConfig(const CommonParamConfig& rhs) = default;
    TLCT_API inline CommonParamConfig& operator=(CommonParamConfig&& rhs) noexcept = default;
    TLCT_API inline CommonParamConfig(CommonParamConfig&& rhs) noexcept = default;
    TLCT_API explicit inline CommonParamConfig(ConfigMap cfg_map) : cfg_map_(std::move(cfg_map)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CommonParamConfig fromPath(const std::string_view& path);

    // Const methods
    [[nodiscard]] TLCT_API inline bool isEmpty() const noexcept;
    [[nodiscard]] TLCT_API inline PipelineType getPipelineType() const noexcept;
    [[nodiscard]] TLCT_API inline const ConfigMap& getConfigMap() const noexcept;

private:
    ConfigMap cfg_map_;
};

CommonParamConfig CommonParamConfig::fromPath(const std::string_view& path)
{
    std::ifstream fs(path.data());
    if (!fs) {
        std::cerr << "Failed to load `" << typeid(CommonParamConfig).name() << "` from `" << path << "`!" << std::endl;
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

    return CommonParamConfig(cfg_map);
}

bool CommonParamConfig::isEmpty() const noexcept { return cfg_map_.empty(); }

PipelineType CommonParamConfig::getPipelineType() const noexcept
{
    const auto it = cfg_map_.find("pipeline");
    if (it == cfg_map_.end()) {
        return PipelineType::RLC;
    }
    const std::string& val = it->second;
    const int ival = std::stoi(val);
    return (PipelineType)ival;
}

const ConfigMap& CommonParamConfig::getConfigMap() const noexcept { return cfg_map_; }

} // namespace tlct::cfg
