#pragma once

#include <algorithm>
#include <cmath>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class PsizeParams_ {
public:
    static constexpr int INVALID_PSIZE = 0;

    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = tcfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] static inline PsizeParams_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg);

    int minPsize;
    int maxPsize;
};

template <tcfg::concepts::CArrange TArrange>
PsizeParams_<TArrange> PsizeParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg) {
    const float safeDiameter = arrange.getDiameter() * SAFE_RATIO;
    const float maxPsizeRatio = (1.f - cvtCfg.viewShiftRange) * SAFE_RATIO / cvtCfg.psizeInflate;
    const int minPsize = _hp::iround(0.1f * safeDiameter);
    const int maxPsize = _hp::iround(maxPsizeRatio * safeDiameter);

    return {minPsize, maxPsize};
}

}  // namespace tlct::_cvt
