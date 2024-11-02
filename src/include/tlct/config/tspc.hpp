#pragma once

#include "tlct/config/tspc/calib.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/config/tspc/param.hpp"
#include "tlct/config/tspc/specific.hpp"

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
