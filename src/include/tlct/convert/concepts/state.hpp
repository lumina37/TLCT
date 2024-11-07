#pragma once

#include <concepts>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/io.hpp"

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
    requires io::concepts::CFrame<typename Self::TFrame>;
    requires requires(const Self self, typename Self::TFrame& dst, int view_row, int view_col) {
        self.renderInto(dst, view_row, view_col);
    };
} && requires {
    // Const methods
    requires requires(Self self) {
        { self.getOutputSize() } -> std::same_as<cv::Size>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const typename Self::TFrame& src) { self.update(src); };
};

} // namespace tlct::_cvt::concepts
