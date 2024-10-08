#pragma once

#include "tspc/calib.hpp"
#include "tspc/layout.hpp"
#include "tspc/param.hpp"
#include "tspc/specific.hpp"

namespace tlct {

namespace cfg::tspc {

namespace _ = _cfg::tspc;

using _::CalibConfig;
using _::Layout;
using _::ParamConfig;
using _::SpecificConfig;

} // namespace cfg::tspc

namespace _cfg {

template class ParamConfig_<tlct::cfg::tspc::Layout>;

}

} // namespace tlct
