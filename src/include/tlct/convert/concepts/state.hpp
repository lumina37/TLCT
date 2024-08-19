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
        tcfg::concepts::CSpecificConfig<typename Self::TSpecificConfig>;
        tcfg::concepts::CLayout<typename Self::TLayout>;
        { Self::fromParamCfg(param_cfg) } -> std::same_as<Self>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const cv::Mat& newsrc) { self.feed(newsrc); };
} && requires {
    // Iterator
    rgs::forward_range<Self>;
    std::convertible_to<decltype(std::declval<Self>().begin()), cv::Mat>;
    std::convertible_to<decltype(std::declval<Self>().end()), cv::Mat>;
};

} // namespace tlct::_cvt::concepts
