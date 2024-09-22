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
    GenericParamConfig() noexcept = delete;
    TLCT_API inline GenericParamConfig(const GenericParamConfig& rhs) = default;
    TLCT_API inline GenericParamConfig& operator=(const GenericParamConfig& rhs) = default;
    TLCT_API inline GenericParamConfig(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig& operator=(GenericParamConfig&& rhs) noexcept = default;
    TLCT_API inline GenericParamConfig(int views, cv::Range range, std::string&& src_pattern,
                                       std::string&& dst_pattern) noexcept
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
    cv::Range range_;
    std::string src_pattern_;
    std::string dst_pattern_;
    int views_;
};

GenericParamConfig GenericParamConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const int views = cfg_map.get("viewNum", 5);
    const int start = cfg_map.get<int>("start_frame");
    const int end = cfg_map.get<int>("end_frame");
    auto src_pattern = cfg_map.get<std::string>("RawImage_Path");
    auto dst_pattern = cfg_map.get<std::string>("Output_Path");
    return {views, {start, end}, std::move(src_pattern), std::move(dst_pattern)};
}

fs::path GenericParamConfig::fmtSrcPath(int i) const noexcept
{
    char buffer[512];
    sprintf(buffer, getSrcPattern().c_str(), i);
    return {buffer};
}

fs::path GenericParamConfig::fmtDstPath(int i) const noexcept
{
    char buffer[512];
    sprintf(buffer, getDstPattern().c_str(), i);
    return {buffer};
}

} // namespace tlct::_cfg
