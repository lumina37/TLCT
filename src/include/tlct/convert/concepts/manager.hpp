#pragma once

#include <concepts>
#include <expected>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::concepts {

namespace tcfg = tlct::cfg;

template <typename Self>
concept CManager = requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const tcfg::CliConfig::Convert& cvtCfg) {
        requires tcfg::concepts::CArrange<typename Self::TArrange>;
        { Self::create(arrange, cvtCfg) } -> std::same_as<std::expected<Self, typename Self::TError>>;
    };
} && requires {
    // Const methods
    requires requires(const Self self, io::YuvPlanarFrame& dst, int viewRow, int viewCol) {
        self.renderInto(dst, viewRow, viewCol);
    };
} && requires {
    // Const methods
    requires requires(Self self) {
        { self.getOutputSize() } -> std::same_as<cv::Size>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const io::YuvPlanarFrame& src) { self.update(src); };
};

}  // namespace tlct::_cvt::concepts
