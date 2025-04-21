#include <algorithm>
#include <expected>
#include <format>
#include <fstream>
#include <map>
#include <string>
#include <utility>

#include "tlct/common/error.hpp"
#include "tlct/helper/charset.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/map.hpp"
#endif

namespace tlct::_cfg {

std::expected<ConfigMap, Error> ConfigMap::createFromFs(std::ifstream&& ifs) {
    const auto isNul = [](const char c) { return c == ' ' || c == '\t'; };

    std::map<std::string, std::string> cfgMap;
    std::string row;
    while (std::getline(ifs, row)) {
        if (row.empty()) [[unlikely]] {
            continue;
        }
        if (row[0] == '#') [[unlikely]] {
            // is comment row
            continue;
        }

        const auto delimIt = std::find(row.begin(), row.end(), ':');
        if (delimIt == row.end()) [[unlikely]] {
            continue;
        }

        const auto keyEndIt = std::find_if(row.begin(), delimIt, isNul);
        if (keyEndIt == delimIt) [[unlikely]] {
            continue;
        }
        const auto valueStartIt = std::find_if_not(delimIt + 1, row.end(), isNul);
        if (valueStartIt == row.end()) [[unlikely]] {
            continue;
        }

#ifdef _WIN32
        const std::string& key = _hp::cconv({row.begin(), keyEndIt});
        const std::string& value = _hp::cconv({valueStartIt, row.end()});
#else
        const std::string_view& key{row.begin(), keyEndIt};
        const std::string_view& value{valueStartIt, row.end()};
#endif
        cfgMap.emplace(key, value);
    }

    return ConfigMap{std::move(cfgMap)};
}

std::expected<ConfigMap, Error> ConfigMap::createFromPath(std::string_view path) {
    std::ifstream ifs(path.data());
    if (!ifs.good()) [[unlikely]] {
        auto errMsg = std::format("Failed to load `ConfigMap` from {}. iostate={}", path, (int)ifs.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, std::move(errMsg)}};
    }

    return createFromFs(std::move(ifs));
}

}  // namespace tlct::_cfg
