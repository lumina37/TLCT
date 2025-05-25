#pragma once

#include <expected>
#include <filesystem>

#include "tlct/common/defines.h"
#include "tlct/helper/error.hpp"

namespace tlct::_cfg {

namespace fs = std::filesystem;

class CliConfig {
public:
    struct Path {
        fs::path src;
        fs::path dst;
    };

    struct Range {
        int begin;
        int end;
    };

    struct Convert {
        int views;
        int upsample;
        float minPsize;
        float psizeInflate;
        float viewShiftRange;
        int psizeShortcutThreshold;
    };

    Path path;
    Range range;
    Convert convert;

    [[nodiscard]] TLCT_API static std::expected<CliConfig, Error> create(const Path& path, const Range& range,
                                                                         const Convert& convert) noexcept;

private:
    CliConfig(Path&& path, const Range& range, const Convert& convert) noexcept;
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.cpp"
#endif
