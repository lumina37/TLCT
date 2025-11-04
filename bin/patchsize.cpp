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

        std::string filename = std::format("v{:03}.bin", fid);
        fs::path psizePath = dstdir / filename;
        manager.getPsizeImpl().dumpRecords(psizePath) | unwrap;
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

    const auto cliCfg = cfgFromCliParser(*parser) | unwrap;
    const auto cfgMap = tlct::ConfigMap::createFromPath(calibFilePath) | unwrap;

    const int pipeline = cfgMap.getOr<"IsMultiFocus">(0);
    const auto& handler = handlers[pipeline];

    handler(cliCfg, cfgMap) | unwrap;
}
