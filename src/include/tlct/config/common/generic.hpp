#pragma once

#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/common/map.hpp"

namespace tlct::_cfg {

namespace fs = std::filesystem;

class GenericParamConfig
{
public:
    // Constructor
    GenericParamConfig() noexcept = delete;
    TLCT_API inline GenericParamConfig(const GenericParamConfig& rhs) = default;
    TLCT_API inline GenericParamConfig& operator=(const GenericParamConfig& rhs) = default;
    TLCT_API inline GenericParamConfig(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig& operator=(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig(int views, cv::Range range, fs::path&& src_path, fs::path&& dst_path) noexcept
        : views_(views), range_(range), src_path_(std::move(src_path)), dst_path_(std::move(dst_path)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline GenericParamConfig fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline int getViews() const noexcept { return views_; };
    [[nodiscard]] TLCT_API inline cv::Range getRange() const noexcept { return range_; };
    [[nodiscard]] TLCT_API inline const fs::path& getSrcPath() const noexcept { return src_path_; };
    [[nodiscard]] TLCT_API inline const fs::path& getDstPath() const noexcept { return dst_path_; };

private:
    fs::path src_path_;
    fs::path dst_path_;
    cv::Range range_;
    int views_;
};

GenericParamConfig GenericParamConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto views = cfg_map.get<"views">(5);
    const auto start = cfg_map.get<"frameBegin", int>();
    const auto end = cfg_map.get<"frameEnd", int>();
    auto src_path = cfg_map.get<"inFile", std::string_view>();
    auto dst_path = cfg_map.get<"outDir", std::string_view>();
    return {views, {start, end}, src_path, dst_path};
}

} // namespace tlct::_cfg
