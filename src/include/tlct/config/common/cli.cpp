#include <memory>
#include <string>
#include <utility>

#include <argparse/argparse.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.hpp"
#endif

namespace tlct::_cfg {

std::unique_ptr<argparse::ArgumentParser> makeUniqArgParser() noexcept {
    auto parser =
        std::make_unique<argparse::ArgumentParser>("tlct", "v" TLCT_VERSION, argparse::default_arguments::all);

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
        .scan<'g', float>()
        .default_value(2.15f);
    parser->add_argument("--viewShiftRange")
        .help("reserve `viewShiftRange*diameter` for view shifting")
        .scan<'g', float>()
        .default_value(0.1f);
    parser->add_argument("--patternSize")
        .help("the size of matching pattern will be `patternSize*diameter`")
        .scan<'g', float>()
        .default_value(0.3f);
    parser->add_argument("--psizeShortcutFactor")
        .help("if the metric of new patch size is smaller than `prevMetric*factor`, then use the prev. one")
        .scan<'g', float>()
        .default_value(1.2f);

    parser->add_epilog(TLCT_COMPILE_INFO);

    return parser;
}

CliConfig CliConfig::fromParser(const argparse::ArgumentParser& parser) {
    auto path = CliConfig::Path{parser.get<std::string>("--src"), parser.get<std::string>("--dst")};
    auto range = CliConfig::Range{parser.get<int>("--begin"), parser.get<int>("--end")};
    auto convert = CliConfig::Convert{parser.get<int>("--views"),          parser.get<int>("--upsample"),
                                      parser.get<float>("--psizeInflate"), parser.get<float>("--viewShiftRange"),
                                      parser.get<float>("--patternSize"),  parser.get<float>("--psizeShortcutFactor")};

    return {std::move(path), std::move(range), std::move(convert)};
}

}  // namespace tlct::_cfg
