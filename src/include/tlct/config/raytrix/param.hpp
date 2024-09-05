#pragma once

#include "calib.hpp"
#include "specific.hpp"
#include "tlct/config/common/param.hpp"

namespace tlct::_cfg::raytrix {

using ParamConfig = ParamConfig_<SpecificConfig, CalibConfig>;

}