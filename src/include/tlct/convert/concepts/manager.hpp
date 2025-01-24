#pragma once

#include <concepts>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::concepts {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <typename Self>
concept CManager = requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg) {
        requires tcfg::concepts::CArrange<typename Self::TArrange>;
        { Self::fromConfigs(arrange, cvtCfg) } -> std::same_as<Self>;
    };
} && requires {
    // Const methods
    requires io::concepts::CFrame<typename Self::TFrame>;
    requires requires(const Self self, typename Self::TFrame& dst, int viewRow, int viewCol) {
        self.renderInto(dst, viewRow, viewCol);
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

}  // namespace tlct::_cvt::concepts
