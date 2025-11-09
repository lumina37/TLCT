#pragma once

#include <vector>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/common/bridge/patch_merge.hpp"
#endif

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange, bool ENABLE_DEBUG>
PatchMergeBridge_<TArrange, ENABLE_DEBUG>::PatchMergeBridge_(const TArrange& arrange, std::vector<TInfo>&& infos,
                                                             std::vector<float>&& weights) noexcept
    : arrange_(arrange), infos_(std::move(infos)), weights_(std::move(weights)) {}

template <cfg::concepts::CArrange TArrange, bool ENABLE_DEBUG>
auto PatchMergeBridge_<TArrange, ENABLE_DEBUG>::create(const TArrange& arrange) noexcept
    -> std::expected<PatchMergeBridge_, Error> {
    std::vector<TInfo> infos;
    infos.resize(arrange.getMIRows() * arrange.getMIMaxCols());

    std::vector<float> weights;
    if (arrange.isMultiFocus()) {
        weights.resize(infos.size());
    }

    return PatchMergeBridge_{arrange, std::move(infos), std::move(weights)};
}

#ifdef _DEBUG
static constexpr bool DEBUG_ENABLED = true;
#else
static constexpr bool DEBUG_ENABLED = false;
#endif

template class PatchMergeBridge_<cfg::CornersArrange, DEBUG_ENABLED>;
template class PatchMergeBridge_<cfg::OffsetArrange, DEBUG_ENABLED>;

}  // namespace tlct::_cvt
