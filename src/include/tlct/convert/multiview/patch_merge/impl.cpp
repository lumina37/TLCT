#include <expected>
#include <ranges>
#include <utility>

#include "../../patchsize/census/impl.hpp"
#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/patch_merge/impl.hpp"
#endif

namespace tlct::_cvt::pm {

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

template class MvImpl_<cfg::CornersArrange>;
template class MvImpl_<cfg::OffsetArrange>;

template std::expected<void, Error> MvImpl_<cfg::CornersArrange>::renderView(
    const census::PsizeImpl_<cfg::CornersArrange>&, io::YuvPlanarFrame&, int, int) const noexcept;
template std::expected<void, Error> MvImpl_<cfg::OffsetArrange>::renderView(
    const census::PsizeImpl_<cfg::OffsetArrange>&, io::YuvPlanarFrame&, int, int) const noexcept;

}  // namespace tlct::_cvt::pm
