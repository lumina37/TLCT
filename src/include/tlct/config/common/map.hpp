#pragma once

#include <concepts>
#include <fstream>
#include <map>
#include <string>
#include <string_view>
#include <utility>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/string.hpp"

namespace tlct::_cfg {

class ConfigMap {
public:
    // Typename alias
    using TMap = std::map<std::string, std::string>;

    // Constructor
    TLCT_API ConfigMap() : map_() {}
    TLCT_API ConfigMap(const ConfigMap& rhs) = default;
    TLCT_API ConfigMap& operator=(const ConfigMap& rhs) = default;
    TLCT_API ConfigMap(ConfigMap&& rhs) noexcept = default;
    TLCT_API ConfigMap& operator=(ConfigMap&& rhs) noexcept = default;
    TLCT_API explicit ConfigMap(TMap&& map) noexcept : map_(std::move(map)){}

    // Initialize from
    [[nodiscard]] TLCT_API static ConfigMap fromFstream(std::ifstream&& ifs);
    [[nodiscard]] TLCT_API static ConfigMap fromPath(std::string_view path);

    // Const methods
    [[nodiscard]] TLCT_API bool isEmpty() const noexcept { return map_.empty(); }

    template <_hp::cestring key, typename Tv>
    [[nodiscard]] Tv get() const;

    template <_hp::cestring key, typename Tv>
        requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
    [[nodiscard]] Tv getOr(const Tv& defaultVal) const noexcept;

    template <_hp::cestring key, typename Tf>
        requires std::is_invocable_v<Tf>
    [[nodiscard]] auto getOrElse(Tf&& defaultFactory) const noexcept -> decltype(defaultFactory());

    template <typename Tv>
    [[nodiscard]] Tv get(const std::string& key) const;

    template <typename Tv>
        requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
    [[nodiscard]] Tv getOr(const std::string& key, const Tv& defaultVal) const noexcept;

    template <typename Tf>
        requires std::is_invocable_v<Tf>
    [[nodiscard]] auto getOrElse(const std::string& key, Tf&& defaultFactory) const noexcept
        -> decltype(defaultFactory());

private:
    TMap map_;
};

template <typename Tv>
static inline Tv stox(const std::string& str) {
    if constexpr (std::is_integral_v<Tv>) {
        return (Tv)std::stoi(str);
    } else if constexpr (std::is_floating_point_v<Tv>) {
        return (Tv)std::stod(str);
    } else {
        return str;
    }
}

template <_hp::cestring key, typename Tv>
Tv ConfigMap::get() const {
    return this->get<Tv>(key.string);
}

template <_hp::cestring key, typename Tv>
    requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
Tv ConfigMap::getOr(const Tv& defaultVal) const noexcept {
    return this->getOr<Tv>(key.string, defaultVal);
}

template <_hp::cestring key, typename Tf>
    requires std::is_invocable_v<Tf>
auto ConfigMap::getOrElse(Tf&& defaultFactory) const noexcept -> decltype(defaultFactory()) {
    return this->getOrElse<Tf>(key.string, defaultFactory);
}

template <typename Tv>
Tv ConfigMap::get(const std::string& key) const {
    return stox<Tv>(map_.at(key));
}

template <typename Tv>
    requires std::is_trivially_copyable_v<Tv> && (!std::is_invocable_v<Tv>)
Tv ConfigMap::getOr(const std::string& key, const Tv& defaultVal) const noexcept {
    const auto it = map_.find(key);
    if (it == map_.end()) {
        return defaultVal;
    }
    const std::string& val = it->second;
    const Tv nval = stox<Tv>(val);
    return nval;
}

template <typename Tf>
    requires std::is_invocable_v<Tf>
auto ConfigMap::getOrElse(const std::string& key, Tf&& defaultFactory) const noexcept -> decltype(defaultFactory()) {
    using Tval = decltype(defaultFactory());

    const auto it = map_.find(key);
    if (it == map_.end()) {
        return defaultFactory();
    }
    const std::string& val = it->second;
    const Tval nval = stox<Tval>(val);
    return nval;
}

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/map.cpp"
#endif
