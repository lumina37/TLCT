#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/bridge.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io/yuv.hpp"

namespace tlct::_cvt::concepts {

template <typename Self, typename TBridge>
concept CMvImpl = requires {
    // Type alias
    requires cfg::concepts::CArrange<typename Self::TArrange>;
} && requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const typename Self::TCvtConfig& cvtCfg,
                      std::shared_ptr<typename Self::TCommonCache> pCommonCache) {
        requires cfg::concepts::CArrange<typename Self::TArrange>;
        { Self::create(arrange, cvtCfg, pCommonCache) } -> std::same_as<std::expected<Self, Error>>;
    };
} && requires {
    // Const methods
    requires requires(const Self self) {
        { self.getOutputSize() } -> std::same_as<cv::Size>;
    };

    requires CBridge<TBridge>;
    requires requires(const Self self, const TBridge& bridge, io::YuvPlanarFrame& dst, int viewRow, int viewCol) {
        self.renderView(bridge, dst, viewRow, viewCol);
    };
};

}  // namespace tlct::_cvt::concepts
