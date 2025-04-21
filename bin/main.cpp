#include <array>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>

#include "tlct.hpp"
#include "tlct_bin_helper.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CManager TManager>
static inline void render(const tlct::CliConfig& cliCfg, const tlct::ConfigMap& map) {
    auto arrange = TManager::TArrange::createWithCfgMap(map).value();
    cv::Size srcSize = arrange.getImgSize();
    arrange.upsample(cliCfg.convert.upsample);

    auto manager = TManager::create(arrange, cliCfg.convert).value();

    cv::Size mvSize = manager.getOutputSize();
    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
        std::swap(mvSize.width, mvSize.height);
    }
    const auto srcExtent = tlct::io::YuvPlanarExtent::createYuv420p8bit(srcSize.width, srcSize.height).value();
    const auto mvExtent = tlct::io::YuvPlanarExtent::createYuv420p8bit(mvSize.width, mvSize.height).value();

    auto yuvReader = tlct::io::YuvPlanarReader::create(cliCfg.path.src, srcExtent).value();

    const fs::path& dstdir = cliCfg.path.dst;
    fs::create_directories(dstdir);
    std::vector<tlct::io::YuvPlanarWriter> yuvWriters;
    const int totalWriters = cliCfg.convert.views * cliCfg.convert.views;
    yuvWriters.reserve(totalWriters);
    for (const int i : rgs::views::iota(0, totalWriters)) {
        std::string filename = std::format("v{:03}-{}x{}.yuv", i, mvSize.width, mvSize.height);
        fs::path savetoPath = dstdir / filename;
        yuvWriters.emplace_back(tlct::io::YuvPlanarWriter::create(savetoPath).value());
    }

    yuvReader.skip(cliCfg.range.begin);

    auto srcFrame = tlct::io::YuvPlanarFrame::create(srcExtent).value();
    auto mvFrame = tlct::io::YuvPlanarFrame::create(mvExtent).value();
    for ([[maybe_unused]] const int fid : rgs::views::iota(cliCfg.range.begin, cliCfg.range.end)) {
        yuvReader.readInto(srcFrame);
        manager.update(srcFrame);

        int view = 0;
        for (const int viewRow : rgs::views::iota(0, cliCfg.convert.views)) {
            for (const int viewCol : rgs::views::iota(0, cliCfg.convert.views)) {
                auto& yuvWriter = yuvWriters[view];
                manager.renderInto(mvFrame, viewRow, viewCol);
                yuvWriter.write(mvFrame);
                view++;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    auto parser = makeUniqArgParser();

    try {
        parser->parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::println(std::cerr, "{}", parser->help().str());
        std::exit(1);
    }

    constexpr std::array handlers{
        render<tlct::raytrix::ManagerYuv420>,
        render<tlct::tspc::ManagerYuv420>,
    };

    try {
        const auto& calibFilePath = parser->get<std::string>("calib_file");
        const auto cliCfg = cfgFromCliParser(*parser).value();
        const auto cfgMap = tlct::ConfigMap::createFromPath(calibFilePath).value();
        const int pipeline = ((cfgMap.getOr<"IsKepler">(0) << 1) | cfgMap.getOr<"IsMultiFocus">(0)) - 1;
        const auto& handler = handlers[pipeline];
        handler(cliCfg, cfgMap);
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::exit(2);
    }
}
