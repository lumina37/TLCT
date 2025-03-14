#pragma once

#include <array>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/multiview/params.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class MvCache_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TArrange = TArrange_;
    using TChannels = std::array<cv::Mat, CHANNELS>;

    // Constructor
    inline MvCache_() noexcept = default;
    inline MvCache_(cv::Mat&& gradBlendingWeight, cv::Mat&& renderCanvas, cv::Mat&& weightCanvas)
        : gradBlendingWeight(std::move(gradBlendingWeight)),
          renderCanvas(std::move(renderCanvas)),
          weightCanvas(std::move(weightCanvas)),
          u8NormedImage() {};
    MvCache_(MvCache_&& rhs) noexcept = default;
    MvCache_& operator=(MvCache_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] static MvCache_ fromParams(const MvParams_<TArrange>& params);

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

template <tcfg::concepts::CArrange TArrange>
MvCache_<TArrange> MvCache_<TArrange>::fromParams(const MvParams_<TArrange>& params) {
    constexpr float GRADIENT_BLENDING_WIDTH = 0.75;
    cv::Mat gradBlendingWeight = circleWithFadeoutBorder(params.resizedPatchWidth, GRADIENT_BLENDING_WIDTH);
    cv::Mat renderCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
    cv::Mat weightCanvas{cv::Size{params.canvasWidth, params.canvasHeight}, CV_32FC1};
    return {std::move(gradBlendingWeight), std::move(renderCanvas), std::move(weightCanvas)};
}

}  // namespace tlct::_cvt
