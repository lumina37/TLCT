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
static inline std::expected<void, tlct::Error> render(const tlct::CliConfig& cliCfg, const tlct::ConfigMap& map) {
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

    {
        auto res = yuvReader.skip(cliCfg.range.begin);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    auto srcFrame = tlct::io::YuvPlanarFrame::create(srcExtent).value();
    auto mvFrame = tlct::io::YuvPlanarFrame::create(mvExtent).value();
    for ([[maybe_unused]] const int fid : rgs::views::iota(cliCfg.range.begin, cliCfg.range.end)) {
        {
            auto res = yuvReader.readInto(srcFrame);
            if (!res) return std::unexpected{std::move(res.error())};
        }
        {
            auto res = manager.update(srcFrame);
            if (!res) return std::unexpected{std::move(res.error())};
        }

        int view = 0;
        for (const int viewRow : rgs::views::iota(0, cliCfg.convert.views)) {
            for (const int viewCol : rgs::views::iota(0, cliCfg.convert.views)) {
                auto& yuvWriter = yuvWriters[view];
                {
                    auto res = manager.renderInto(mvFrame, viewRow, viewCol);
                    if (!res) return std::unexpected{std::move(res.error())};
                }
                {
                    auto res = yuvWriter.write(mvFrame);
                    if (!res) return std::unexpected{std::move(res.error())};
                }
                view++;
            }
        }
    }

    return {};
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
        render<tlct::tspc::ManagerYuv420>,
        render<tlct::raytrix::ManagerYuv420>,
    };

    std::string calibFilePath;
    try {
        calibFilePath = parser->get<std::string>("calib_file");
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::exit(1);
    }

    const auto cliCfgRes = cfgFromCliParser(*parser);
    if (!cliCfgRes) [[unlikely]] {
        std::println(std::cerr, "{}", cliCfgRes.error().msg);
        std::exit(1);
    }
    const auto& cliCfg = cliCfgRes.value();

    const auto cfgMapRes = tlct::ConfigMap::createFromPath(calibFilePath);
    if (!cfgMapRes) [[unlikely]] {
        std::println(std::cerr, "{}", cfgMapRes.error().msg);
        std::exit(1);
    }
    const auto& cfgMap = cfgMapRes.value();

    const int pipeline = cfgMap.getOr<"IsMultiFocus">(0);
    const auto& handler = handlers[pipeline];

    try {
        handler(cliCfg, cfgMap);
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::exit(1);
    }
}
