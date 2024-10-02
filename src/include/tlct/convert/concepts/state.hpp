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
        requires tcfg::concepts::CLayout<typename Self::TLayout>;
        { Self::fromParamCfg(param_cfg) } -> std::same_as<Self>;
    };
} && requires {
    // Const methods
    requires requires(const Self self, cv::Mat& dst, int view_row, int view_col) {
        self.renderInto(dst, view_row, view_col);
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const cv::Mat& src) { self.feed(src); };
};

} // namespace tlct::_cvt::concepts
