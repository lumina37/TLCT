#include <array>
#include <cstdlib>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <vector>

#include "tlct.hpp"
#include "tlct_cli.hpp"
#include "tlct_unwrap.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CManager TManager>
static std::expected<void, tlct::Error> render(const tlct::CliConfig& cliCfg, const tlct::ConfigMap& map) noexcept {
    auto arrangeRes = TManager::TArrange::createWithCfgMap(map);
    if (!arrangeRes) return std::unexpected{std::move(arrangeRes.error())};
    auto& arrange = arrangeRes.value();

    cv::Size srcSize = arrange.getImgSize();
    arrange.upsample(cliCfg.convert.upsample);

    auto managerRes = TManager::create(arrange, cliCfg.convert);
    if (!managerRes) return std::unexpected{std::move(managerRes.error())};
    auto& manager = managerRes.value();

    cv::Size mvSize = manager.getOutputSize();
    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
        std::swap(mvSize.width, mvSize.height);
    }

    auto srcExtentRes = tlct::io::YuvPlanarExtent::createYuv420p8bit(srcSize.width, srcSize.height);
    if (!srcExtentRes) return std::unexpected{std::move(srcExtentRes.error())};
    auto srcExtent = srcExtentRes.value();

    auto mvExtentRes = tlct::io::YuvPlanarExtent::createYuv420p8bit(mvSize.width, mvSize.height);
    if (!mvExtentRes) return std::unexpected{std::move(mvExtentRes.error())};
    auto mvExtent = mvExtentRes.value();

    auto yuvReaderRes = tlct::io::YuvPlanarReader::create(cliCfg.path.src, srcExtent);
    if (!yuvReaderRes) return std::unexpected{std::move(yuvReaderRes.error())};
    auto& yuvReader = yuvReaderRes.value();

    const fs::path& dstdir = cliCfg.path.dst;
    fs::create_directories(dstdir);
    std::vector<tlct::io::YuvPlanarWriter> yuvWriters;
    const int totalWriters = cliCfg.convert.views * cliCfg.convert.views;
    yuvWriters.reserve(totalWriters);
    for (const int i : rgs::views::iota(0, totalWriters)) {
        std::string filename = std::format("v{:03}-{}x{}.yuv", i, mvSize.width, mvSize.height);
        fs::path savetoPath = dstdir / filename;
        auto yuvWriterRes = tlct::io::YuvPlanarWriter::create(savetoPath);
        if (!yuvWriterRes) return std::unexpected{std::move(yuvWriterRes.error())};
        yuvWriters.push_back(std::move(yuvWriterRes.value()));
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
        render<tlct::cvt::TSPCMeth0Manager>,
        render<tlct::cvt::RaytrixMeth0Manager>,
        render<tlct::cvt::TSPCMeth1Manager>,
        render<tlct::cvt::RaytrixMeth1Manager>,
    };

    std::string calibFilePath;
    try {
        calibFilePath = parser->get<std::string>("calib_file");
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::exit(1);
    }

    const auto cliCfg = cfgFromCliParser(*parser) | unwrap;
    const auto cfgMap = tlct::ConfigMap::createFromPath(calibFilePath) | unwrap;

    const int pipeline = cliCfg.convert.method * 2 + cfgMap.getOr<"IsMultiFocus">(0);
    const auto& handler = handlers[pipeline];

    handler(cliCfg, cfgMap) | unwrap;
}
