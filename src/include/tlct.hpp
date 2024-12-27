#pragma once

#include "tlct/config.hpp"
#include "tlct/convert.hpp"
#include "tlct/helper.hpp"
#include "tlct/io.hpp"
#include "tlct/namespace.hpp"

#ifdef TLCT_BUILD_APP

namespace tlct {

using cfg::CliConfig;
using cfg::makeParser;

} // namespace tlct

#endif

namespace tlct {

using cfg::ConfigMap;

} // namespace tlct
