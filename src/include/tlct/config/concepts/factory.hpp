#pragma once

#include <concepts>

namespace tlct::_cfg::concepts {

template <typename Tf, typename Tv>
concept is_factory_of = requires(Tf factory) {
    { factory() } -> std::same_as<Tv>;
};

} // namespace tlct::_cfg::concepts
