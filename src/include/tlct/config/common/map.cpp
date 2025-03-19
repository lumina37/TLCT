#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "tlct/helper/charset.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/map.hpp"
#endif

namespace tlct::_cfg {

ConfigMap ConfigMap::fromFstream(std::ifstream&& ifs) {
    const auto is_nul = [](const char c) { return c == ' ' || c == '\t'; };

    std::map<std::string, std::string> cfg_map;
    std::string row;
    while (std::getline(ifs, row)) {
        if (row.empty()) [[unlikely]] {
            continue;
        }
        if (row[0] == '#') [[unlikely]] {
            // is comment row
            continue;
        }

        const auto delim_it = std::find(row.begin(), row.end(), ':');
        if (delim_it == row.end()) [[unlikely]] {
            continue;
        }

        const auto key_end_it = std::find_if(row.begin(), delim_it, is_nul);
        if (key_end_it == delim_it) [[unlikely]] {
            continue;
        }
        const auto value_start_it = std::find_if_not(delim_it + 1, row.end(), is_nul);
        if (value_start_it == row.end()) [[unlikely]] {
            continue;
        }

#ifdef _WIN32
        const std::string& key = _hp::cconv({row.begin(), key_end_it});
        const std::string& value = _hp::cconv({value_start_it, row.end()});
#else
        const std::string_view& key{row.begin(), key_end_it};
        const std::string_view& value{value_start_it, row.end()};
#endif
        cfg_map.emplace(key, value);
    }

    return ConfigMap(std::move(cfg_map));
}

ConfigMap ConfigMap::fromPath(std::string_view path) {
    std::ifstream ifs(path.data());
    if (!ifs) [[unlikely]] {
        std::stringstream err;
        err << "Failed to load `ConfigMap`, the file may not exist. path: " << path;
        throw std::runtime_error{err.str()};
    }

    return fromFstream(std::move(ifs));
}

}  // namespace tlct::_cfg
