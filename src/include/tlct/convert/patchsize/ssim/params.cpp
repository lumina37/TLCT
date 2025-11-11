#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/params.hpp"
#endif

namespace tlct::_cvt::ssim {

template <cfg::concepts::CArrange TArrange>
auto PsizeParams_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<PsizeParams_, Error> {
    constexpr float PATTERN_SIZE = 0.35f;

    const float patternSize = arrange.getDiameter() * PATTERN_SIZE;
    const float radius = arrange.getDiameter() / 2.f;
    const float halfPatternSize = patternSize / 2.f;
    const float patternShift = std::sqrt((radius - halfPatternSize) * (radius + halfPatternSize)) - halfPatternSize;

    const int minPsize = (int)std::roundf(0.75f * patternSize);

    return PsizeParams_{patternSize, patternShift, minPsize, cvtCfg.psizeShortcutThreshold};
}

template class PsizeParams_<cfg::CornersArrange>;
template class PsizeParams_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::ssim
