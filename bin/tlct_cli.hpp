#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include <argparse/argparse.hpp>
#include <tlct.hpp>

[[nodiscard]] static std::unique_ptr<argparse::ArgumentParser> makeUniqArgParser() noexcept {
    auto parser = std::make_unique<argparse::ArgumentParser>("tlct", std::string("v").append(tlct::version),
                                                             argparse::default_arguments::all);

    parser->set_usage_max_line_width(120);
    parser->add_argument("calib_file").help("path of the `calib.cfg`").required();
    parser->add_group("I/O");
    parser->add_argument("-i", "--src").help("input yuv420p file").required();
    parser->add_argument("-o", "--dst").help("output directory").required();
    parser->add_group("Frame Range");
    parser->add_argument("-b", "--begin")
        .help("the index of the start frame, left inclusive, starts from zero")
        .scan<'i', int>()
        .default_value(0);
    parser->add_argument("-e", "--end")
        .help("the index of the end frame, right exclusive")
        .scan<'i', int>()
        .default_value(1);
    parser->add_group("Conversion");
    parser->add_argument("--views").help("viewpoint number").scan<'i', int>().default_value(1);
    parser->add_argument("--upsample")
        .help("the input image will be upsampled by this scale")
        .scan<'i', int>()
        .default_value(1);
    parser->add_argument("--minPsize")
        .help("min patch size is `diameter*minPsize`")
        .scan<'g', float>()
        .default_value(0.2f);
    parser->add_argument("--psizeInflate")
        .help("the extracted patch will be inflated by this scale")
        .scan<'g', float>()
        .default_value(2.15f);
    parser->add_argument("--viewShiftRange")
        .help("reserve `viewShiftRange*diameter` for view shifting")
        .scan<'g', float>()
        .default_value(0.1f);
    parser->add_argument("--psizeShortcutThreshold")
        .help("if the bit diff of `dhash` is smaller than this threshold, then use the prev. patchsize")
        .scan<'i', int>()
        .default_value(4);

    parser->add_epilog(std::string{tlct::compileInfo});

    return parser;
}

[[nodiscard]] static std::expected<tlct::CliConfig, tlct::Error> cfgFromCliParser(
    const argparse::ArgumentParser& parser) noexcept {
    const tlct::CliConfig::Path path{parser.get<std::string>("--src"), parser.get<std::string>("--dst")};
    const tlct::CliConfig::Range range{parser.get<int>("--begin"), parser.get<int>("--end")};
    const tlct::CliConfig::Convert convert{parser.get<int>("--views"),
                                           parser.get<int>("--upsample"),
                                           parser.get<float>("--minPsize"),
                                           parser.get<float>("--psizeInflate"),
                                           parser.get<float>("--viewShiftRange"),
                                           parser.get<int>("--psizeShortcutThreshold")};
    return tlct::CliConfig::create(path, range, convert);
}