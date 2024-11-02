#pragma once

#include "tlct/config/raytrix/calib.hpp"
#include "tlct/config/raytrix/layout.hpp"
#include "tlct/config/raytrix/param.hpp"
#include "tlct/config/raytrix/specific.hpp"

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
