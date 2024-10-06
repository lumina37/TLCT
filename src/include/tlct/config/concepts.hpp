#pragma once

#include "concepts/calib.hpp"
#include "concepts/factory.hpp"
#include "concepts/layout.hpp"
#include "concepts/specific.hpp"

namespace tlct::cfg::concepts {

namespace _ = _cfg::concepts;

using _::CCalibConfig;
using _::CLayout;
using _::CSpecificConfig;
using _::is_factory_of;

} // namespace tlct::cfg::concepts
