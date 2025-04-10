#include <cassert>
#include <numbers>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.hpp"
#endif

namespace tlct::_cfg {

CliConfig::CliConfig(Path path, Range range, Convert convert) : path(path), range(range), convert(convert) {
    assert(range.end > range.begin);

    assert(convert.views > 0);
    assert(convert.upsample > 0);
    assert(convert.minPsize > 0.0f);
    assert(convert.minPsize < 1.0f);
    assert(convert.psizeInflate >= std::numbers::sqrt3_v<float>);
    assert(convert.psizeInflate < 3.0f);
    assert(convert.viewShiftRange >= 0.0f);
    assert(convert.viewShiftRange < 1.0f);
    assert(convert.psizeShortcutFactor >= 0.0f);
}

}  // namespace tlct::_cfg
