#pragma once

#include <cstdio>
#include <string>

#include <opencv2/core.hpp>

#include "cfg_map.hpp"
#include "generic.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/config/raytrix/calib.hpp"
#include "tlct/config/raytrix/specific.hpp"
#include "tlct/config/tspc/calib.hpp"
#include "tlct/config/tspc/specific.hpp"

namespace tlct::_cfg {

template <typename TSpecificConfig_, typename TCalibConfig_>
    requires concepts::CSpecificConfig<TSpecificConfig_> && concepts::CCalibConfig<TCalibConfig_>
class ParamConfig_
{
public:
    // Typename alias
    using TSpecificConfig = TSpecificConfig_;
    using TCalibConfig = TCalibConfig_;

    // Constructor
    TLCT_API inline ParamConfig_() noexcept : generic_cfg_(), spec_cfg_(), calib_cfg_(){};
    TLCT_API inline ParamConfig_(const ParamConfig_& rhs) noexcept = default;
    TLCT_API inline ParamConfig_& operator=(const ParamConfig_& rhs) noexcept = default;
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

    // Non-const methods
    TLCT_API inline ParamConfig_& setCalibCfg(const TCalibConfig& cfg) noexcept
    {
        calib_cfg_ = cfg;
        return *this;
    };

private:
    GenericParamConfig generic_cfg_;
    TSpecificConfig spec_cfg_;
    TCalibConfig calib_cfg_;
};

template <typename TSpecificConfig, typename TCalibConfig>
    requires concepts::CSpecificConfig<TSpecificConfig> && concepts::CCalibConfig<TCalibConfig>
ParamConfig_<TSpecificConfig, TCalibConfig>
ParamConfig_<TSpecificConfig, TCalibConfig>::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    auto generic_cfg = GenericParamConfig::fromConfigMap(cfg_map);
    auto spec_cfg = TSpecificConfig::fromConfigMap(cfg_map);
    auto calib_cfg = TCalibConfig::fromXMLPath(map.at("Calibration_xml"));
    return {std::move(generic_cfg), std::move(spec_cfg), std::move(calib_cfg)};
}

template class ParamConfig_<tspc::SpecificConfig, tspc::CalibConfig>;
template class ParamConfig_<raytrix::SpecificConfig, raytrix::CalibConfig>;

} // namespace tlct::_cfg
