#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/convert/concepts/bridge.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::concepts {

template <typename Self>
concept CPsizeImpl = requires {
    // Type alias
    requires CBridge<typename Self::TBridge>;
} && requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const typename Self::TCvtConfig& cvtCfg) {
        requires cfg::concepts::CArrange<typename Self::TArrange>;
        { Self::create(arrange, cvtCfg) } -> std::same_as<std::expected<Self, Error>>;
    };
} && requires {
    // Non-const methods
    requires requires(Self self, const cv::Mat& src, typename Self::TBridge& bridge) {
        self.updateBridge(src, bridge);
    };
};

}  // namespace tlct::_cvt::concepts
