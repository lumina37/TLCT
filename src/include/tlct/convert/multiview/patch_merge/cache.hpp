#pragma once

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/multiview/params.hpp"

namespace tlct::_cvt::pm {

template <cfg::concepts::CArrange TArrange_>
class MvCache_ {
public:
    // Typename alias
    using TArrange = TArrange_;
    using TMvParams = MvParams_<TArrange>;

private:
    MvCache_(cv::Mat&& gradBlendingWeight, cv::Mat&& renderCanvas, cv::Mat&& weightCanvas) noexcept;

public:
    // Constructor
    MvCache_() noexcept = default;
    MvCache_(const MvCache_& rhs) = delete;
    MvCache_& operator=(const MvCache_& rhs) = delete;
    MvCache_(MvCache_&& rhs) noexcept = default;
    MvCache_& operator=(MvCache_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MvCache_, Error> create(const TMvParams& params) noexcept;

    cv::Mat gradBlendingWeight;
    cv::Mat renderCanvas;
    cv::Mat weightCanvas;

    cv::Mat f32Chan;
    cv::Mat u8NormedImage;
};

}  // namespace tlct::_cvt::pm

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/patch_merge/cache.cpp"
#endif
