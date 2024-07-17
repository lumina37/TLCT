#pragma once

#include "tspc/calib.hpp"
#include "tspc/layout.hpp"
#include "tspc/specific.hpp"

namespace tlct::cfg::tspc {

namespace _priv = tlct::_cfg::tspc;

using _priv::CalibConfig;
using _priv::Layout;
using _priv::SpecificConfig;

using ParamConfig = _cfg::ParamConfig_<SpecificConfig, CalibConfig>;

} // namespace tlct::cfg::tspc
