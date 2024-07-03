#pragma once

#include "raytrix/calib.hpp"
#include "raytrix/layout.hpp"
#include "raytrix/specific.hpp"

namespace tlct::cfg::raytrix {

using ParamConfig = ParamConfig_<SpecificConfig, CalibConfig>;

}
