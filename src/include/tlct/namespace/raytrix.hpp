#pragma once

#include "tlct/config/raytrix.hpp"
#include "tlct/convert/raytrix.hpp"

namespace tlct::raytrix {

using cfg::raytrix::CalibConfig;
using cfg::raytrix::Layout;
using cfg::raytrix::LEN_TYPE_NUM;
using cfg::raytrix::ParamConfig;
using cfg::raytrix::SpecificConfig;

using cvt::raytrix::FarNeighbors;
using cvt::raytrix::NearNeighbors;
using cvt::raytrix::State_;
using cvt::raytrix::StateYuv420;

} // namespace tlct::raytrix
