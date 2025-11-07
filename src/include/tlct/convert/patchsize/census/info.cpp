#pragma once

#include <vector>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/census/info.hpp"
#endif

namespace tlct::_cvt::census {

template <cfg::concepts::CArrange TArrange, bool ENABLE_DEBUG>
PatchInfos_<TArrange, ENABLE_DEBUG>::PatchInfos_(const TArrange& arrange, std::vector<TPatchInfo>&& infos,
                                                 std::vector<float>&& weights) noexcept
    : arrange_(arrange), infoVec_(std::move(infos)), weights_(std::move(weights)) {}

template <cfg::concepts::CArrange TArrange, bool ENABLE_DEBUG>
auto PatchInfos_<TArrange, ENABLE_DEBUG>::create(const TArrange& arrange) noexcept
    -> std::expected<PatchInfos_, Error> {
    std::vector<TPatchInfo> infos;
    infos.resize(arrange.getMIRows() * arrange.getMIMaxCols());

    std::vector<float> weights;
    if (arrange.isMultiFocus()) {
        weights.resize(infos.size());
    }

    return PatchInfos_{arrange, std::move(infos), std::move(weights)};
}

#ifdef _DEBUG
static constexpr bool DEBUG_ENABLED = true;
#else
static constexpr bool DEBUG_ENABLED = false;
#endif

template class PatchInfos_<cfg::CornersArrange, DEBUG_ENABLED>;
template class PatchInfos_<cfg::OffsetArrange, DEBUG_ENABLED>;

}  // namespace tlct::_cvt::census
