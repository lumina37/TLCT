#pragma once

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
    [[nodiscard]] static inline PsizeParams_ fromConfigs(const TArrange& arrange, const TCvtConfig& cvt_cfg);

    float pattern_size;
    float pattern_shift;
    int min_psize;
};

template <tcfg::concepts::CArrange TArrange>
PsizeParams_<TArrange> PsizeParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvt_cfg) {
    const float pattern_size = arrange.getDiameter() * cvt_cfg.pattern_size;
    const float radius = arrange.getDiameter() / 2.f;
    const float half_pattern_size = pattern_size / 2.f;
    const float max_pattern_shift =
        std::sqrt((radius - half_pattern_size) * (radius + half_pattern_size)) - half_pattern_size;
    const float candidate_pattern_shift = radius * cvt_cfg.max_psize;
    const float pattern_shift = std::min(max_pattern_shift, candidate_pattern_shift);

    const int min_psize = (int)std::roundf(0.75f * pattern_size);

    return {pattern_size, pattern_shift, min_psize};
}

}  // namespace tlct::_cvt
