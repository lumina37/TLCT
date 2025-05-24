#include <expected>

#include "tlct/config/arrange.hpp"
#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/params.hpp"
#endif

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange>
auto PsizeParams_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<PsizeParams_, Error> {
    const float safeDiameter = arrange.getDiameter() * CONTENT_SAFE_RATIO;
    const float maxPsizeRatio = (1.f - cvtCfg.viewShiftRange) * CONTENT_SAFE_RATIO / cvtCfg.psizeInflate;
    const int minPsize = _hp::iround(cvtCfg.minPsize * arrange.getDiameter());
    const int maxPsize = _hp::iround(maxPsizeRatio * safeDiameter);

    return PsizeParams_{minPsize, maxPsize, cvtCfg.psizeShortcutFactor};
}

template class PsizeParams_<cfg::CornersArrange>;
template class PsizeParams_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
