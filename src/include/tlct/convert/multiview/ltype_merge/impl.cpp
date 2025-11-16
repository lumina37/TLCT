#include <ranges>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/common/bridge/patch_merge.hpp"
#include "tlct/convert/concepts/multiview.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io/yuv.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/ltype_merge/impl.hpp"
#endif

namespace tlct::_cvt::lm {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
MvImpl_<TArrange>::MvImpl_(const TArrange& arrange, const TMvParams& params, TMvCache&& cache,
                           std::shared_ptr<TCommonCache>&& pCommonCache) noexcept
    : arrange_(arrange), params_(params), mvCache_(std::move(cache)), pCommonCache_(pCommonCache) {}

template <cfg::concepts::CArrange TArrange>
auto MvImpl_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg,
                               std::shared_ptr<TCommonCache> pCommonCache) noexcept -> std::expected<MvImpl_, Error> {
    auto paramRes = TMvParams::create(arrange, cvtCfg);
    if (!paramRes) return std::unexpected{std::move(paramRes.error())};
    auto& params = paramRes.value();

    auto mvCacheRes = TMvCache::create(params);
    if (!mvCacheRes) return std::unexpected{std::move(mvCacheRes.error())};
    auto& mvCache = mvCacheRes.value();

    return MvImpl_{arrange, params, std::move(mvCache), std::move(pCommonCache)};
}

static_assert(concepts::CMvImpl<MvImpl_<cfg::CornersArrange>, PatchMergeBridge_<cfg::CornersArrange>>);
template class MvImpl_<cfg::CornersArrange>;

static_assert(concepts::CMvImpl<MvImpl_<cfg::OffsetArrange>, PatchMergeBridge_<cfg::OffsetArrange>>);
template class MvImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::lm
