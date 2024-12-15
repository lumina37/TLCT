#pragma once

#include <toml++/toml.hpp>

#include "tlct/common/defines.h"

namespace tlct::_hp {

TLCT_API inline int getPipeline(const toml::table& table) noexcept { return table["pipeline"].value_or(0); }

} // namespace tlct::_hp
