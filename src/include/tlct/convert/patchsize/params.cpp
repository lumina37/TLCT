#include "tlct/config/arrange.hpp"
#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/consts.hpp"
#include "tlct/helper/constexpr/math.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/params.hpp"
#endif

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange>
PsizeParams_<TArrange> PsizeParams_<TArrange>::fromConfigs(const TArrange& arrange, const TCvtConfig& cvtCfg) {
    const float safeDiameter = arrange.getDiameter() * CONTENT_SAFE_RATIO;
    const float maxPsizeRatio = (1.f - cvtCfg.viewShiftRange) * CONTENT_SAFE_RATIO / cvtCfg.psizeInflate;
    const int minPsize = _hp::iround(cvtCfg.minPsize * arrange.getDiameter());
    const int maxPsize = _hp::iround(maxPsizeRatio * safeDiameter);

    return {minPsize, maxPsize};
}

template class PsizeParams_<_cfg::CornersArrange>;
template class PsizeParams_<_cfg::OffsetArrange>;

}  // namespace tlct::_cvt
