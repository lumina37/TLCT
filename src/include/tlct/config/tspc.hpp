#pragma once

#include "tlct/config/concepts.hpp"
#include "tlct/config/tspc/calib.hpp"
#include "tlct/config/tspc/layout.hpp"
#include "tlct/config/tspc/param.hpp"
#include "tlct/config/tspc/specific.hpp"

namespace tlct {

namespace cfg::tspc {

namespace _ = _cfg::tspc;

using _::ParamConfig;

using _::SpecificConfig;
static_assert(concepts::CSpecificConfig<SpecificConfig>);

using _::CalibConfig;
static_assert(concepts::CCalibConfig<CalibConfig>);

using _::Layout;
static_assert(concepts::CLayout<Layout>);

} // namespace cfg::tspc

namespace _cfg {

template class ParamConfig_<tlct::cfg::tspc::Layout>;

}

} // namespace tlct
