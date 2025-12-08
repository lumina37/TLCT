#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"
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
    const float safeDiameter = arrange.getDiameter() * CONTENT_SAFE_RATIO;

    const float safeRadius = safeDiameter / 2.f;
    const float halfPatternSize = patternSize / 2.f;
    const float safePatternShift =
        std::sqrt((safeRadius - halfPatternSize) * (safeRadius + halfPatternSize)) - halfPatternSize;

    const float maxPsizeRatio = (1.f - cvtCfg.viewShiftRange) * CONTENT_SAFE_RATIO / cvtCfg.psizeInflate;
    const float candiPatternShift = maxPsizeRatio * safeDiameter / 2.f;

    const float patternShift = std::min(safePatternShift, candiPatternShift);

    const int minPsize = _hp::iround(0.5f * patternSize);

    return PsizeParams_{patternSize, patternShift, minPsize, cvtCfg.psizeShortcutThreshold};
}

template class PsizeParams_<cfg::CornersArrange>;
template class PsizeParams_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::ssim
