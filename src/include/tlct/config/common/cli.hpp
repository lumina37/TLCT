#pragma once

#include <filesystem>
#include <memory>

#include <argparse/argparse.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cfg {

namespace fs = std::filesystem;

[[nodiscard]] TLCT_API inline std::unique_ptr<argparse::ArgumentParser> makeUniqArgParser() noexcept
{
    auto parser =
        std::make_unique<argparse::ArgumentParser>("tlct", "v" tlct_VERSION, argparse::default_arguments::all);

    parser->set_usage_max_line_width(120);
    parser->add_argument("calib_file").help("path of the `calib.cfg`").required();
    parser->add_group("I/O");
    parser->add_argument("-i", "--src").help("input yuv420p file").required();
    parser->add_argument("-o", "--dst").help("output directory").required();
    parser->add_group("Frame Range");
    parser->add_argument("-b", "--begin")
        .help("the index of the start frame, left contains, starts from zero")
        .scan<'i', int>()
        .default_value(0);
    parser->add_argument("-e", "--end")
        .help("the index of the end frame, right NOT contains")
        .scan<'i', int>()
        .default_value(1);
    parser->add_group("Conversion");
    parser->add_argument("--views").help("viewpoint number").scan<'i', int>().default_value(1);
    parser->add_argument("--upsample")
        .help("the input image will be upsampled by this scale")
        .scan<'i', int>()
        .default_value(1);
    parser->add_argument("--psizeInflate")
        .help("the extracted patch will be inflated by this scale")
        .scan<'g', double>()
        .default_value(2.15);
    parser->add_argument("--viewShiftRange")
        .help("reserve `viewShiftRange*diameter` for view shifting")
        .scan<'g', double>()
        .default_value(0.1);
    parser->add_argument("--patternSize")
        .help("the size of matching pattern will be `patternSize*diameter`")
        .scan<'g', double>()
        .default_value(0.3);
    parser->add_argument("--psizeShortcutThre")
        .help("if the difference bit of dhash of MI is smaller than this value, then use the prev. patch size")
        .scan<'i', int>()
        .default_value(4);

    parser->add_epilog(TLCT_COMPILE_INFO);

    return parser;
}

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
        inline Convert(int views, int upsample, double psize_inflate, double view_shift_range, double pattern_size,
                       int psize_shortcut_threshold) noexcept
            : views(views), upsample(upsample), psize_inflate(psize_inflate), view_shift_range(view_shift_range),
              max_psize((1.0 - view_shift_range) / psize_inflate), pattern_size(pattern_size),
              psize_shortcut_threshold(psize_shortcut_threshold) {};

        int views;
        int upsample;
        double psize_inflate;
        double view_shift_range;
        double max_psize;
        double pattern_size;
        int psize_shortcut_threshold;
    };

    Path path;
    Range range;
    Convert convert;

    // Initialize from
    [[nodiscard]] TLCT_API static inline CliConfig fromParser(const argparse::ArgumentParser& parser);
};

CliConfig CliConfig::fromParser(const argparse::ArgumentParser& parser)
{
    auto path = CliConfig::Path{parser.get<std::string>("--src"), parser.get<std::string>("--dst")};
    auto range = CliConfig::Range{parser.get<int>("--begin"), parser.get<int>("--end")};
    auto convert = CliConfig::Convert{parser.get<int>("--views"),           parser.get<int>("--upsample"),
                                      parser.get<double>("--psizeInflate"), parser.get<double>("--viewShiftRange"),
                                      parser.get<double>("--patternSize"),  parser.get<int>("--psizeShortcutThre")};

    return {std::move(path), std::move(range), std::move(convert)};
}

} // namespace tlct::_cfg
