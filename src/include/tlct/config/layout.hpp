#pragma once

#include "tlct/config/concepts.hpp"
#include "tlct/config/layout/corners.hpp"
#include "tlct/config/layout/offset.hpp"

namespace tlct::cfg {

namespace _ = _cfg;

using _::CornersLayout;
static_assert(concepts::CLayout<CornersLayout>);

using _::OffsetLayout;
static_assert(concepts::CLayout<OffsetLayout>);

} // namespace tlct::cfg
