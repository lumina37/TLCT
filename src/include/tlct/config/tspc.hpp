#pragma once

#include "tlct/config/concepts.hpp"
#include "tlct/config/tspc/layout.hpp"

namespace tlct::cfg::tspc {

namespace _ = _cfg::tspc;

using _::Layout;
static_assert(concepts::CLayout<Layout>);

} // namespace tlct::cfg::tspc
