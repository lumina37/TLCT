#include <expected>
#include <ranges>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/psize.hpp"
#include "tlct/convert/patchsize/impl.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/impl.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
MvImpl_<TArrange>::MvImpl_(const TArrange& arrange, const MvParams& params, MvCache&& cache) noexcept
    : arrange_(arrange), params_(params), cache_(std::move(cache)) {}

template <cfg::concepts::CArrange TArrange>
auto MvImpl_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<MvImpl_, Error> {
    auto paramRes = MvParams::create(arrange, cvtCfg);
    if (!paramRes) return std::unexpected{std::move(paramRes.error())};
    auto& params = paramRes.value();

    auto mvCacheRes = MvCache::create(params);
    if (!mvCacheRes) return std::unexpected{std::move(mvCacheRes.error())};
    auto& mvCache = mvCacheRes.value();

    return MvImpl_{arrange, params, std::move(mvCache)};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> MvImpl_<TArrange>::update(const io::YuvPlanarFrame& src) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    src.getY().copyTo(cache_.rawSrcs[0]);
    src.getU().copyTo(cache_.rawSrcs[1]);
    src.getV().copyTo(cache_.rawSrcs[2]);

    if (arrange_.getDirection()) {
        for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
            cv::transpose(cache_.rawSrcs[i], cache_.rawSrcs[i]);
        }
    }

    const int upsample = arrange_.getUpsample();
    if (upsample != 1) [[likely]] {
        cv::resize(cache_.rawSrcs[0], cache_.srcs[0], {}, upsample, upsample, cv::INTER_LINEAR_EXACT);
    } else {
        cache_.srcs[0] = cache_.rawSrcs[0];
    }

    if (src.getExtent().getUShift() != 0) {
        const int uUpsample = upsample << src.getExtent().getUShift();
        cv::resize(cache_.rawSrcs[1], cache_.srcs[1], {}, uUpsample, uUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        cache_.srcs[1] = cache_.rawSrcs[1];
    }

    if (src.getExtent().getVShift() != 0) {
        const int vUpsample = upsample << src.getExtent().getVShift();
        cv::resize(cache_.rawSrcs[2], cache_.srcs[2], {}, vUpsample, vUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        cache_.srcs[2] = cache_.rawSrcs[2];
    }

    return {};
}

template class MvImpl_<cfg::CornersArrange>;
template class MvImpl_<cfg::OffsetArrange>;

template std::expected<void, Error> MvImpl_<cfg::CornersArrange>::renderView(const PsizeImpl_<cfg::CornersArrange>&,
                                                                             int, int) const noexcept;
template std::expected<void, Error> MvImpl_<cfg::OffsetArrange>::renderView(const PsizeImpl_<cfg::OffsetArrange>&, int,
                                                                            int) const noexcept;

}  // namespace tlct::_cvt
