#pragma once

#include <filesystem>
#include <string>

#include <argparse/argparse.hpp>
#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cfg {

namespace fs = std::filesystem;

struct CommonConfig {
    struct Path {
        fs::path src;
        fs::path dst;
    };

    struct Range {
        int begin;
        int end;
    };

    struct Convert {
        inline Convert(int views, int upsample, double psize_inflate, double max_psize, double pattern_size,
                       int psize_shortcut_threshold) noexcept
            : views(views), upsample(upsample), psize_inflate(psize_inflate),
              max_psize(std::min(max_psize, 1.0 / psize_inflate)), pattern_size(pattern_size),
              psize_shortcut_threshold(psize_shortcut_threshold) {};

        int views;
        int upsample;
        double psize_inflate;
        double max_psize;
        double pattern_size;
        int psize_shortcut_threshold;
    };

    Path path;
    Range range;
    Convert convert;

    // Initialize from
    [[nodiscard]] TLCT_API static inline CommonConfig fromParser(const argparse::ArgumentParser& parser);
};

CommonConfig CommonConfig::fromParser(const argparse::ArgumentParser& parser)
{
    auto path = CommonConfig::Path{parser.get<std::string>("--src"), parser.get<std::string>("--dst")};
    auto range = CommonConfig::Range{parser.get<int>("--begin"), parser.get<int>("--end")};
    auto convert = CommonConfig::Convert{parser.get<int>("--views"),           parser.get<int>("--upsample"),
                                         parser.get<double>("--psizeInflate"), parser.get<double>("--maxPsize"),
                                         parser.get<double>("--patternSize"),  parser.get<int>("--psizeShortcutThre")};

    return {std::move(path), std::move(range), std::move(convert)};
}

} // namespace tlct::_cfg
