#pragma once

#include "tlct/common/defines.h"

#ifdef TLCT_BUILD_APP

#    include "tlct/config/common/cli.hpp"

namespace tlct::cfg {

namespace _ = _cfg;

using _::CliConfig;
using _::makeParser;

} // namespace tlct::cfg

#endif

#include "tlct/config/common/map.hpp"

namespace tlct::cfg {

using _cfg::ConfigMap;

} // namespace tlct::cfg
