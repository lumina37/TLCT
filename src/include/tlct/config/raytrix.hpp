#pragma once

#include "raytrix/calib.hpp"
#include "raytrix/layout.hpp"
#include "raytrix/param.hpp"
#include "raytrix/specific.hpp"

namespace tlct {

namespace cfg::raytrix {

namespace _ = _cfg::raytrix;

using _::CalibConfig;
using _::Layout;
using _::LEN_TYPE_NUM;
using _::ParamConfig;
using _::SpecificConfig;

} // namespace cfg::raytrix

namespace _cfg {

template class ParamConfig_<tlct::cfg::raytrix::Layout>;

}

} // namespace tlct
