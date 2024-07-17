#pragma once

#include <concepts>

#include "tlct/config/common/cfg_map.hpp"

namespace tlct::_cfg::concepts {

template <typename Self>
concept CSpecificConfig = std::copyable<Self> && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires(const ConfigMap& cfg_map) {
    // Init from
    { Self::fromConfigMap(cfg_map) } -> std::same_as<Self>;
};

} // namespace tlct::_cfg::concepts
