#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt::concepts {

template <typename Self>
concept CManager = requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const cfg::CliConfig::Convert& cvtCfg) {
        requires cfg::concepts::CArrange<typename Self::TArrange>;
        { Self::create(arrange, cvtCfg) } -> std::same_as<std::expected<Self, Error>>;
    };
} && requires {
    // Const methods
    requires requires(Self self) {
        { self.getOutputSize() } -> std::same_as<cv::Size>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const io::YuvPlanarFrame& src) { self.update(src); };
    requires requires(Self self, io::YuvPlanarFrame& dst, int viewRow, int viewCol) {
        self.renderInto(dst, viewRow, viewCol);
    };
};

}  // namespace tlct::_cvt::concepts
