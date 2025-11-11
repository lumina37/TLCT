#include <array>
#include <cstdlib>
#include <exception>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <print>
#include <string>

#include <argparse/argparse.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"
#include "tlct_unwrap.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

[[nodiscard]] static std::unique_ptr<argparse::ArgumentParser> makeUniqArgParser() noexcept {
    auto parser = std::make_unique<argparse::ArgumentParser>("tlct", std::string("v").append(tlct::version),
                                                             argparse::default_arguments::all);

    parser->set_usage_max_line_width(120);
    parser->add_argument("calib_file").help("path of the `calib.cfg`").required();
    parser->add_argument("x").help("col").scan<'i', int>().required();
    parser->add_argument("y").help("row").scan<'i', int>().required();
    parser->add_group("I/O");
    parser->add_argument("-i", "--src").help("input yuv420p file").required();
    parser->add_argument("-o", "--dst").help("output directory").required();
    parser->add_epilog(std::string{tlct::compileInfo});

    return parser;
}

template <tlct::concepts::CManager TManager>
static std::expected<void, tlct::Error> paint(std::unique_ptr<argparse::ArgumentParser>& pParser,
                                              const tlct::ConfigMap& map) noexcept {
    auto arrangeRes = TManager::TArrange::createWithCfgMap(map);
    if (!arrangeRes) return std::unexpected{std::move(arrangeRes.error())};
    auto& arrange = arrangeRes.value();

    cv::Size srcSize = arrange.getImgSize();
    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
    }

    auto srcExtentRes = tlct::io::YuvPlanarExtent::createYuv420p8bit(srcSize.width, srcSize.height);
    if (!srcExtentRes) return std::unexpected{std::move(srcExtentRes.error())};
    auto srcExtent = srcExtentRes.value();

    const fs::path srcPath{pParser->get<std::string>("--src")};
    auto yuvReaderRes = tlct::io::YuvPlanarReader::create(srcPath, srcExtent);
    if (!yuvReaderRes) return std::unexpected{std::move(yuvReaderRes.error())};
    auto& yuvReader = yuvReaderRes.value();

    const fs::path dstDir{pParser->get<std::string>("--dst")};
    fs::create_directories(dstDir);

    auto srcFrame = tlct::io::YuvPlanarFrame::create(srcExtent).value();
    {
        auto res = yuvReader.readInto(srcFrame);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    auto canvas = srcFrame.getY().clone();
    if (arrange.getDirection()) {
        cv::transpose(canvas, canvas);
    }

    const int miIndexX = pParser->get<int>("x");
    const int miIndexY = pParser->get<int>("y");
    auto miCenter = arrange.getMICenter(miIndexY, miIndexX);
    cv::circle(canvas, miCenter, (int)arrange.getRadius(), cv::Scalar::all(255.));

    if (arrange.getDirection()) {
        cv::transpose(canvas, canvas);
    }

    cv::imwrite((dstDir / "canvas.png").string(), canvas);

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
        paint<tlct::cvt::TSPCMeth0Manager>,
        paint<tlct::cvt::RaytrixMeth0Manager>,
        paint<tlct::cvt::TSPCMeth1Manager>,
        paint<tlct::cvt::RaytrixMeth1Manager>,
    };

    std::string calibFilePath;
    try {
        calibFilePath = parser->get<std::string>("calib_file");
    } catch (const std::exception& err) {
        std::println(std::cerr, "{}", err.what());
        std::exit(1);
    }

    const auto cfgMap = tlct::ConfigMap::createFromPath(calibFilePath) | unwrap;

    const int pipeline = cfgMap.getOr<"IsMultiFocus">(0);
    const auto& handler = handlers[pipeline];

    handler(parser, cfgMap) | unwrap;
}
