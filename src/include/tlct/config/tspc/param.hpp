#pragma once

#include <cstdio>
#include <filesystem>
#include <string>

#include <opencv2/core.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/common.hpp"
#include "tlct/config/concepts/calib.hpp"

namespace tlct::cfg::tspc {

namespace fs = std::filesystem;

template <typename TCalibConfig_>
    requires concepts::CCalibConfig<TCalibConfig_>
class ParamConfig
{
public:
    // Typename alias
    using TCalibConfig = TCalibConfig_;

    // Constructor
    TLCT_API inline ParamConfig() noexcept
        : calib_cfg_(), views_(), imgsize_(), range_(), src_pattern_(), dst_pattern_(){};
    TLCT_API inline ParamConfig& operator=(const ParamConfig& rhs) noexcept = default;
    TLCT_API inline ParamConfig(const ParamConfig& rhs) noexcept = default;
    TLCT_API inline ParamConfig& operator=(ParamConfig&& rhs) noexcept = default;
    TLCT_API inline ParamConfig(ParamConfig&& rhs) noexcept = default;
    TLCT_API inline ParamConfig(TCalibConfig calib_cfg, int views, cv::Size imgsize, cv::Range range,
                                std::string src_pattern, std::string dst_pattern) noexcept
        : calib_cfg_(calib_cfg), views_(views), imgsize_(imgsize), range_(range), src_pattern_(std::move(src_pattern)),
          dst_pattern_(std::move(dst_pattern)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ParamConfig fromCommonCfg(const CommonParamConfig& cfg);

    // Const methods
    [[nodiscard]] TLCT_API inline const TCalibConfig& getCalibCfg() const noexcept { return calib_cfg_; };
    [[nodiscard]] TLCT_API inline int getViews() const noexcept { return views_; };
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline cv::Range getRange() const noexcept { return range_; };
    [[nodiscard]] TLCT_API inline const std::string& getSrcPattern() const noexcept { return src_pattern_; };
    [[nodiscard]] TLCT_API inline const std::string& getDstPattern() const noexcept { return dst_pattern_; };

    // Utils
    [[nodiscard]] TLCT_API static inline fs::path fmtSrcPath(const ParamConfig& cfg, int i) noexcept;
    [[nodiscard]] TLCT_API static inline fs::path fmtDstPath(const ParamConfig& cfg, int i) noexcept;

private:
    TCalibConfig calib_cfg_;
    int views_;
    cv::Size imgsize_;
    cv::Range range_;
    std::string src_pattern_;
    std::string dst_pattern_;
};

template <typename TCalibConfig>
    requires concepts::CCalibConfig<TCalibConfig>
ParamConfig<TCalibConfig> ParamConfig<TCalibConfig>::fromCommonCfg(const CommonParamConfig& cfg)
{
    const auto& cfg_map = cfg.getConfigMap();
    auto calib_cfg = TCalibConfig::fromXMLPath(cfg_map.at("Calibration_xml"));
    const int views = std::stoi(cfg_map.at("viewNum"));
    const int width = std::stoi(cfg_map.at("width"));
    const int height = std::stoi(cfg_map.at("height"));
    const int start = std::stoi(cfg_map.at("start_frame"));
    const int end = std::stoi(cfg_map.at("end_frame"));
    const std::string& src_pattern = cfg_map.at("RawImage_Path");
    const std::string& dst_pattern = cfg_map.at("Output_Path");
    return {calib_cfg, views, {width, height}, {start, end}, src_pattern, dst_pattern};
}

template <typename TCalibConfig>
    requires concepts::CCalibConfig<TCalibConfig>
fs::path ParamConfig<TCalibConfig>::fmtSrcPath(const ParamConfig& cfg, int i) noexcept
{
    char buffer[256];
    sprintf(buffer, cfg.getSrcPattern().c_str(), i);
    return {buffer};
}

template <typename TCalibConfig>
    requires concepts::CCalibConfig<TCalibConfig>
fs::path ParamConfig<TCalibConfig>::fmtDstPath(const ParamConfig& cfg, int i) noexcept
{
    char buffer[256];
    sprintf(buffer, cfg.getDstPattern().c_str(), i);
    return {buffer};
}

template class ParamConfig<CalibConfig>;

} // namespace tlct::cfg::tspc
