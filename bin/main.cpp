#include <array>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
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

    auto manager = TManager::fromConfigs(arrange, cliCfg.convert);

    cv::Size outputSize = manager.getOutputSize();
    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
        std::swap(outputSize.width, outputSize.height);
    }

    using TYuvReader = tlct::io::YuvReader_<typename TManager::TFrame>;
    using TYuvWriter = tlct::io::YuvWriter_<typename TManager::TFrame>;
    auto yuvReader = TYuvReader::fromPath(cliCfg.path.src, srcSize.width, srcSize.height);

    const fs::path& dstdir = cliCfg.path.dst;
    fs::create_directories(dstdir);
    std::vector<TYuvWriter> yuvWriters;
    const int totalWriters = cliCfg.convert.views * cliCfg.convert.views;
    yuvWriters.reserve(totalWriters);
    for (const int i : rgs::views::iota(0, totalWriters)) {
        std::stringstream filenameStream;
        filenameStream << 'v' << std::setw(3) << std::setfill('0') << i << '-' << outputSize.width << 'x'
                       << outputSize.height << ".yuv";
        fs::path savetoPath = dstdir / filenameStream.str();
        yuvWriters.emplace_back(TYuvWriter::fromPath(savetoPath));
    }

    yuvReader.skip(cliCfg.range.begin);

    auto srcFrame = typename TManager::TFrame{srcSize};
    auto mvFrame = typename TManager::TFrame{outputSize};
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
        std::cerr << err.what() << std::endl;
        std::cerr << *parser;
        std::exit(1);
    }

    constexpr std::array handlers{
        render<tlct::raytrix::ManagerYuv420>,
        render<tlct::tspc::ManagerYuv420>,
    };

    try {
        const auto& calibFilePath = parser->get<std::string>("calib_file");
        const auto& cliCfg = cfgFromCliParser(*parser);
        const auto& cfgMap = tlct::ConfigMap::fromPath(calibFilePath);
        const int pipeline = ((cfgMap.getOr<"IsKepler">(0) << 1) | cfgMap.getOr<"IsMultiFocus">(0)) - 1;
        const auto& handler = handlers[pipeline];
        handler(cliCfg, cfgMap);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::exit(2);
    }
}
