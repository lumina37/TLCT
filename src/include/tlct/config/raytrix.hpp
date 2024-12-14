#pragma once

#include "tlct/config/concepts.hpp"
#include "tlct/config/raytrix/layout.hpp"

namespace tlct::cfg::raytrix {

namespace _ = _cfg::raytrix;

using _::LEN_TYPE_NUM;

using _::Layout;
static_assert(concepts::CLayout<Layout>);

} // namespace tlct::cfg::raytrix
