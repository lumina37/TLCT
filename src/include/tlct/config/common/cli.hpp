#pragma once

#include <filesystem>

#include "tlct/common/defines.h"

namespace tlct::_cfg {

namespace fs = std::filesystem;

struct CliConfig {
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
        float psizeShortcutFactor;
    };

    Path path;
    Range range;
    Convert convert;

    TLCT_API CliConfig(Path path, Range range, Convert convert);
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.cpp"
#endif
