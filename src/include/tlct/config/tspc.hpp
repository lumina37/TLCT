#pragma once

#include "tspc/calib.hpp"
#include "tspc/layout.hpp"
#include "tspc/specific.hpp"

namespace tlct::cfg::tspc {

using ParamConfig = ParamConfig_<SpecificConfig, CalibConfig>;

}
