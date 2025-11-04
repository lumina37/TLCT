#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange_>
class CommonCache_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TArrange = TArrange_;
    using TChannels = std::array<cv::Mat, CHANNELS>;

    // Constructor
    CommonCache_() noexcept = default;
    CommonCache_(CommonCache_&& rhs) noexcept = default;
    CommonCache_& operator=(CommonCache_&& rhs) noexcept = default;
    CommonCache_(const TArrange& arrange) noexcept;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<CommonCache_, Error> create(const TArrange& arrange) noexcept;

    [[nodiscard]] TLCT_API std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;

    TChannels rawSrcs;
    TChannels srcs;

private:
    const TArrange& arrange_;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/common/cache.cpp"
#endif
