#pragma once

#include <array>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/multiview/params.hpp"

namespace tlct::_cvt {

template <tcfg::concepts::CArrange TArrange_>
class MvCache_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TArrange = TArrange_;
    using TChannels = std::array<cv::Mat, CHANNELS>;

    // Constructor
    MvCache_() noexcept = default;
    MvCache_(cv::Mat&& gradBlendingWeight, cv::Mat&& renderCanvas, cv::Mat&& weightCanvas)
        : gradBlendingWeight(std::move(gradBlendingWeight)),
          renderCanvas(std::move(renderCanvas)),
          weightCanvas(std::move(weightCanvas)),
          u8NormedImage() {}
    MvCache_(MvCache_&& rhs) noexcept = default;
    MvCache_& operator=(MvCache_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static MvCache_ fromParams(const MvParams_<TArrange>& params);

    cv::Mat gradBlendingWeight;
    cv::Mat renderCanvas;
    cv::Mat weightCanvas;

    TChannels rawSrcs;
    TChannels srcs;
    TChannels f32Srcs;

    cv::Mat normedImage;
    cv::Mat u8NormedImage;
    cv::Mat weights;
    TChannels u8OutputImageChannels;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/cache.cpp"
#endif
