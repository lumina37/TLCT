#pragma once

#include "raytrix/calib.hpp"
#include "raytrix/layout.hpp"
#include "raytrix/specific.hpp"

namespace tlct::cfg::raytrix {

namespace _priv = tlct::_cfg::raytrix;

using _priv::CalibConfig;
using _priv::Layout;
using _priv::LEN_TYPE_NUM;
using _priv::SpecificConfig;

using ParamConfig = _cfg::ParamConfig_<SpecificConfig, CalibConfig>;

} // namespace tlct::cfg::raytrix
