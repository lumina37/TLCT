#pragma once

#include <algorithm>
#include <cmath>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"

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

    float patternSize;
    float patternShift;
    int minPsize;
};

template <tcfg::concepts::CArrange TArrange>
PsizeParams_<TArrange> PsizeParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg) {
    const float patternSize = arrange.getDiameter() * cvtCfg.patternSize;
    const float radius = arrange.getDiameter() / 2.f;
    const float safeRadius = radius * 0.95f;
    const float halfPatternSize = patternSize / 2.f;
    const float maxPatternShift =
        std::sqrt((safeRadius - halfPatternSize) * (safeRadius + halfPatternSize)) - halfPatternSize;
    const float candidatePatternShift = radius * cvtCfg.maxPsize;
    const float patternShift = std::min(maxPatternShift, candidatePatternShift);

    const int minPsize = (int)std::roundf(0.75f * patternSize);

    return {patternSize, patternShift, minPsize};
}

}  // namespace tlct::_cvt
