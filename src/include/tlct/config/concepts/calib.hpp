#pragma once

#include <concepts>

#include <pugixml.hpp>

namespace tlct::_cfg::concepts {

template <typename Self>
concept CCalibConfig = std::copyable<Self> && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires {
    // Init from
    requires requires(const pugi::xml_document& doc) {
        { Self::fromXMLDoc(doc) } -> std::same_as<Self>;
    };
    requires requires(const std::string_view& path) {
        { Self::fromXMLPath(path) } -> std::same_as<Self>;
    };
};

} // namespace tlct::_cfg::concepts
