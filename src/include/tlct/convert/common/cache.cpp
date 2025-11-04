#include <expected>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/common/cache.hpp"
#endif

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
CommonCache_<TArrange>::CommonCache_(const TArrange& arrange) noexcept : arrange_(arrange) {}

template <cfg::concepts::CArrange TArrange>
auto CommonCache_<TArrange>::create(const TArrange& arrange) noexcept -> std::expected<CommonCache_, Error> {
    // TODO: the memory alloc should be moved here
    return {arrange};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> CommonCache_<TArrange>::update(const io::YuvPlanarFrame& src) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    src.getY().copyTo(rawSrcs[0]);
    src.getU().copyTo(rawSrcs[1]);
    src.getV().copyTo(rawSrcs[2]);

    if (arrange_.getDirection()) {
        for (const int i : rgs::views::iota(0, CHANNELS)) {
            cv::transpose(rawSrcs[i], rawSrcs[i]);
        }
    }

    const int upsample = arrange_.getUpsample();
    if (upsample != 1) [[likely]] {
        cv::resize(rawSrcs[0], srcs[0], {}, upsample, upsample, cv::INTER_LINEAR);
    } else {
        srcs[0] = rawSrcs[0];
    }

    if (src.getExtent().getUShift() != 0) {
        const int uUpsample = upsample << src.getExtent().getUShift();
        cv::resize(rawSrcs[1], srcs[1], {}, uUpsample, uUpsample, cv::INTER_LINEAR);
    } else {
        srcs[1] = rawSrcs[1];
    }

    if (src.getExtent().getVShift() != 0) {
        const int vUpsample = upsample << src.getExtent().getVShift();
        cv::resize(rawSrcs[2], srcs[2], {}, vUpsample, vUpsample, cv::INTER_LINEAR);
    } else {
        srcs[2] = rawSrcs[2];
    }

    return {};
}

template class CommonCache_<cfg::CornersArrange>;
template class CommonCache_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt
