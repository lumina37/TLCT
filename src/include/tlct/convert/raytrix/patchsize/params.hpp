#pragma once

#include <cmath>

#include "tlct/config/concepts.hpp"
#include "tlct/config/raytrix.hpp"

namespace tlct::_cvt::raytrix {

namespace tcfg = tlct::cfg::raytrix;

class PsizeParams
{
public:
    static constexpr int INVALID_PSIZE = 0;

    // Typename alias
    using TLayout = tcfg::Layout;
    using TSpecificConfig = TLayout::TSpecificConfig;

    // Initialize from
    [[nodiscard]] static inline PsizeParams fromConfigs(const TLayout& layout, const TSpecificConfig& spec_cfg);

    double pattern_size;
    double pattern_shift;
    int min_psize;
};

PsizeParams PsizeParams::fromConfigs(const TLayout& layout, const TSpecificConfig& spec_cfg)
{
    const double pattern_size = layout.getDiameter() * spec_cfg.getPatternSize();
    const double radius = layout.getDiameter() / 2.0;
    const double half_pattern_size = pattern_size / 2.0;
    const double max_pattern_shift =
        std::sqrt((radius - half_pattern_size) * (radius + half_pattern_size)) - half_pattern_size;
    const double candidate_pattern_shift = radius * spec_cfg.getMaxPsize();
    const double pattern_shift = std::min(max_pattern_shift, candidate_pattern_shift);

    const int min_psize = (int)std::round(0.3 * pattern_size);

    return {pattern_size, pattern_shift, min_psize};
}

} // namespace tlct::_cvt::raytrix
