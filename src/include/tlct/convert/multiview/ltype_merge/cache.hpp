#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/multiview/params.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::lm {

template <cfg::concepts::CArrange TArrange_>
class MvCache_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TArrange = TArrange_;
    using TMvParams = MvParams_<TArrange>;

private:
    MvCache_(cv::Mat&& renderCanvas, cv::Mat&& weightCanvas, cv::Mat&& gradsWeightCanvas,
             std::array<cv::Mat, CHANNELS>&& lenTypeWeights) noexcept;

public:
    // Constructor
    MvCache_() noexcept = default;
    MvCache_(const MvCache_& rhs) = delete;
    MvCache_& operator=(const MvCache_& rhs) = delete;
    MvCache_(MvCache_&& rhs) noexcept = default;
    MvCache_& operator=(MvCache_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MvCache_, Error> create(const TMvParams& params) noexcept;

    cv::Mat renderCanvas;
    cv::Mat weightCanvas;
    cv::Mat gradsWeightCanvas;
    std::array<cv::Mat, CHANNELS> lenTypeWeights;

    cv::Mat f32Chan;
    cv::Mat u8NormedImage;
};

}  // namespace tlct::_cvt::lm

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/ltype_merge/cache.cpp"
#endif
