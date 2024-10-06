#pragma once

#include <cstdio>
#include <string>

#include <opencv2/core.hpp>

#include "cfg_map.hpp"
#include "generic.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/config/raytrix/layout.hpp"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::_cfg {

template <typename TLayout_>
    requires concepts::CLayout<TLayout_>
class ParamConfig_
{
public:
    // Typename alias
    using TLayout = TLayout_;
    using TSpecificConfig = TLayout::TSpecificConfig;
    using TCalibConfig = TLayout::TCalibConfig;

    // Constructor
    ParamConfig_() = delete;
    TLCT_API inline ParamConfig_(const ParamConfig_& rhs) = default;
    TLCT_API inline ParamConfig_& operator=(const ParamConfig_& rhs) = default;
    TLCT_API inline ParamConfig_(ParamConfig_&& rhs) noexcept = default;
    TLCT_API inline ParamConfig_& operator=(ParamConfig_&& rhs) noexcept = default;
    TLCT_API inline ParamConfig_(GenericParamConfig&& generic_cfg, TSpecificConfig&& spec_cfg,
                                 TCalibConfig&& calib_cfg) noexcept
        : generic_cfg_(std::move(generic_cfg)), spec_cfg_(std::move(spec_cfg)), calib_cfg_(std::move(calib_cfg)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline ParamConfig_ fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline const GenericParamConfig& getGenericCfg() const noexcept { return generic_cfg_; };
    [[nodiscard]] TLCT_API inline const TSpecificConfig& getSpecificCfg() const noexcept { return spec_cfg_; };
    [[nodiscard]] TLCT_API inline const TCalibConfig& getCalibCfg() const noexcept { return calib_cfg_; };

private:
    GenericParamConfig generic_cfg_;
    TSpecificConfig spec_cfg_;
    TCalibConfig calib_cfg_;
};

template <typename TLayout>
    requires concepts::CLayout<TLayout>
ParamConfig_<TLayout> ParamConfig_<TLayout>::fromConfigMap(const ConfigMap& cfg_map)
{
    auto generic_cfg = GenericParamConfig::fromConfigMap(cfg_map);
    auto spec_cfg = TSpecificConfig::fromConfigMap(cfg_map);
    auto calib_cfg = TCalibConfig::fromXMLPath(cfg_map.get<std::string, "Calibration_xml">());
    return {std::move(generic_cfg), std::move(spec_cfg), std::move(calib_cfg)};
}

} // namespace tlct::_cfg
