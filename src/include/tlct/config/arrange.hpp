#pragma once

#include "tlct/config/arrange/corners.hpp"
#include "tlct/config/arrange/offset.hpp"
#include "tlct/config/concepts.hpp"

namespace tlct::cfg {

namespace _ = _cfg;

using _::CornersArrange;
static_assert(concepts::CArrange<CornersArrange>);

using _::OffsetArrange;
static_assert(concepts::CArrange<OffsetArrange>);

}  // namespace tlct::cfg
