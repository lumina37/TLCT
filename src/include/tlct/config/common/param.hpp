#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

#include "cfg_map.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts/calib.hpp"

namespace tlct::cfg {

namespace fs = std::filesystem;

class CommonParamConfig
{
public:
    // Constructor
    TLCT_API inline CommonParamConfig() noexcept : views_(), range_(), src_pattern_(), dst_pattern_(){};
    TLCT_API inline CommonParamConfig& operator=(const CommonParamConfig& rhs) noexcept = default;
    TLCT_API inline CommonParamConfig(const CommonParamConfig& rhs) noexcept = default;
    TLCT_API inline CommonParamConfig& operator=(CommonParamConfig&& rhs) noexcept = default;
    TLCT_API inline CommonParamConfig(CommonParamConfig&& rhs) noexcept = default;
    TLCT_API inline CommonParamConfig(int views, cv::Range range, std::string src_pattern,
                                      std::string dst_pattern) noexcept
        : views_(views), range_(range), src_pattern_(std::move(src_pattern)), dst_pattern_(std::move(dst_pattern)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CommonParamConfig fromConfigMap(const ConfigMap& cfg_map);

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

CommonParamConfig CommonParamConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    const int views = std::stoi(map.at("viewNum"));
    const int start = std::stoi(map.at("start_frame"));
    const int end = std::stoi(map.at("end_frame"));
    const std::string& src_pattern = map.at("RawImage_Path");
    const std::string& dst_pattern = map.at("Output_Path");
    return {views, {start, end}, src_pattern, dst_pattern};
}

fs::path CommonParamConfig::fmtSrcPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, getSrcPattern().c_str(), i);
    return {buffer};
}

fs::path CommonParamConfig::fmtDstPath(int i) const noexcept
{
    char buffer[256];
    sprintf(buffer, getDstPattern().c_str(), i);
    return {buffer};
}

template <typename TCalibConfig_>
    requires concepts::CCalibConfig<TCalibConfig_>
class ParamConfig_
{
public:
    // Typename alias
    using TCalibConfig = TCalibConfig_;

    // Constructor
    TLCT_API inline ParamConfig_() noexcept : calib_cfg_(), common_cfg_(), imgsize_(){};
    TLCT_API inline ParamConfig_& operator=(const ParamConfig_& rhs) noexcept = default;
    TLCT_API inline ParamConfig_(const ParamConfig_& rhs) noexcept = default;
    TLCT_API inline ParamConfig_& operator=(ParamConfig_&& rhs) noexcept = default;
    TLCT_API inline ParamConfig_(ParamConfig_&& rhs) noexcept = default;
    TLCT_API inline ParamConfig_(TCalibConfig calib_cfg, CommonParamConfig common_cfg, cv::Size imgsize) noexcept
        : calib_cfg_(calib_cfg), common_cfg_(std::move(common_cfg)), imgsize_(imgsize){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ParamConfig_ fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline const TCalibConfig& getCalibCfg() const noexcept { return calib_cfg_; };
    [[nodiscard]] TLCT_API inline const CommonParamConfig& getCommonCfg() const noexcept { return common_cfg_; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };

private:
    CommonParamConfig common_cfg_;
    TCalibConfig calib_cfg_;
    cv::Size imgsize_;
};

template <typename TCalibConfig>
    requires concepts::CCalibConfig<TCalibConfig>
ParamConfig_<TCalibConfig> ParamConfig_<TCalibConfig>::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    const auto calib_cfg = TCalibConfig::fromXMLPath(map.at("Calibration_xml"));
    const auto common_cfg = CommonParamConfig::fromConfigMap(cfg_map);
    const int width = std::stoi(map.at("width"));
    const int height = std::stoi(map.at("height"));
    return {calib_cfg, common_cfg, {width, height}};
}

} // namespace tlct::cfg
