#pragma once

#include <memory>

#include <argparse/argparse.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cfg {

[[nodiscard]] TLCT_API inline std::unique_ptr<argparse::ArgumentParser> newParser() noexcept
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

} // namespace tlct::_cfg
