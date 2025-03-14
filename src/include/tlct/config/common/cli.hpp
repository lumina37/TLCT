#pragma once

#include <filesystem>
#include <memory>

#include <argparse/argparse.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cfg {

namespace fs = std::filesystem;

[[nodiscard]] TLCT_API std::unique_ptr<argparse::ArgumentParser> makeUniqArgParser() noexcept;

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
        TLCT_API Convert(int views, int upsample, float psizeInflate, float viewShiftRange, float patternSize,
                         float psizeShortcutFactor) noexcept
            : views(views),
              upsample(upsample),
              psizeInflate(psizeInflate),
              viewShiftRange(viewShiftRange),
              maxPsize((1.f - viewShiftRange) / psizeInflate),
              patternSize(patternSize),
              psizeShortcutFactor(psizeShortcutFactor) {};

        int views;
        int upsample;
        float psizeInflate;
        float viewShiftRange;
        float maxPsize;
        float patternSize;
        float psizeShortcutFactor;
    };

    Path path;
    Range range;
    Convert convert;

    // Initialize from
    [[nodiscard]] TLCT_API static CliConfig fromParser(const argparse::ArgumentParser& parser);
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.cpp"
#endif
