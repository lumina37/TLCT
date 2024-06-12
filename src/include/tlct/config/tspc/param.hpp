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
    TLCT_API inline ParamConfig() noexcept : calib_cfg_(), common_cfg_(), imgsize_(){};
    TLCT_API inline ParamConfig& operator=(const ParamConfig& rhs) noexcept = default;
    TLCT_API inline ParamConfig(const ParamConfig& rhs) noexcept = default;
    TLCT_API inline ParamConfig& operator=(ParamConfig&& rhs) noexcept = default;
    TLCT_API inline ParamConfig(ParamConfig&& rhs) noexcept = default;
    TLCT_API inline ParamConfig(TCalibConfig calib_cfg, CommonParamConfig common_cfg, cv::Size imgsize) noexcept
        : calib_cfg_(calib_cfg), common_cfg_(common_cfg), imgsize_(imgsize){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ParamConfig fromConfigMap(const ConfigMap& cfg_map);

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
ParamConfig<TCalibConfig> ParamConfig<TCalibConfig>::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    const auto calib_cfg = TCalibConfig::fromXMLPath(map.at("Calibration_xml"));
    const auto common_cfg = CommonParamConfig::fromConfigMap(cfg_map);
    const int width = std::stoi(map.at("width"));
    const int height = std::stoi(map.at("height"));
    return {calib_cfg, common_cfg, {width, height}};
}

template class ParamConfig<CalibConfig>;

} // namespace tlct::cfg::tspc
