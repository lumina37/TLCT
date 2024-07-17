#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

#include "cfg_map.hpp"
#include "tlct/common/defines.h"

namespace tlct::_cfg {

namespace fs = std::filesystem;

class GenericParamConfig
{
public:
    // Constructor
    TLCT_API inline GenericParamConfig() noexcept : views_(), range_(), src_pattern_(), dst_pattern_(){};
    TLCT_API inline GenericParamConfig& operator=(const GenericParamConfig& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig(const GenericParamConfig& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig& operator=(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig(int views, cv::Range range, std::string src_pattern,
                                       std::string dst_pattern) noexcept
        : views_(views), range_(range), src_pattern_(std::move(src_pattern)), dst_pattern_(std::move(dst_pattern)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline GenericParamConfig fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline int getViews() const noexcept { return views_; };
    [[nodiscard]] TLCT_API inline cv::Range getRange() const noexcept { return range_; };
    [[nodiscard]] TLCT_API inline const std::string& getSrcPattern() const noexcept { return src_pattern_; };
    [[nodiscard]] TLCT_API inline const std::string& getDstPattern() const noexcept { return dst_pattern_; };

    [[nodiscard]] TLCT_API inline fs::path fmtSrcPath(int i) const noexcept;
    [[nodiscard]] TLCT_API inline fs::path fmtDstPath(int i) const noexcept;

private:
    int views_;
    cv::Range range_;
    std::string src_pattern_;
    std::string dst_pattern_;
};

GenericParamConfig GenericParamConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    const int views = std::stoi(map.at("viewNum"));
    const int start = std::stoi(map.at("start_frame"));
    const int end = std::stoi(map.at("end_frame"));
    const std::string& src_pattern = map.at("RawImage_Path");
    const std::string& dst_pattern = map.at("Output_Path");
    return {views, {start, end}, src_pattern, dst_pattern};
}

fs::path GenericParamConfig::fmtSrcPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, getSrcPattern().c_str(), i);
    return {buffer};
}

fs::path GenericParamConfig::fmtDstPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, getDstPattern().c_str(), i);
    return {buffer};
}

} // namespace tlct::_cfg
