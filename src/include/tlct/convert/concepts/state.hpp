#pragma once

#include <concepts>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt::concepts {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <typename Self>
concept CState = requires {
    // Initialize from
    requires requires(const Self::TParamConfig& param_cfg) {
        requires tcfg::concepts::CSpecificConfig<typename Self::TSpecificConfig>;
        requires tcfg::concepts::CLayout<typename Self::TLayout>;
        { Self::fromParamCfg(param_cfg) } -> std::same_as<Self>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const cv::Mat& newsrc) { self.feed(newsrc); };
} && requires {
    // Iterator
    requires requires(Self self) {
        { *self.begin() } -> std::same_as<cv::Mat>;
        { *self.end() } -> std::same_as<cv::Mat>;
    };
};

} // namespace tlct::_cvt::concepts
