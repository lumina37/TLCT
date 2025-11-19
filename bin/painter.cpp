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

static void bgr2yuv(const cv::Mat& src, tlct::io::YuvPlanarFrame& frame) {
    const auto& extent = frame.getExtent();

    cv::Mat yuvImage;
    cv::cvtColor(src, yuvImage, cv::COLOR_BGR2YUV_I420);

    const uint8_t* yuvData = yuvImage.data;
    std::memcpy(frame.getY().data, yuvData, extent.getYByteSize());
    std::memcpy(frame.getU().data, yuvData + extent.getYByteSize(), extent.getUByteSize());
    std::memcpy(frame.getV().data, yuvData + extent.getYByteSize() + extent.getUByteSize(), extent.getVByteSize());
}

template <tlct::concepts::CManager TManager>
static std::expected<void, tlct::Error> paintFrame(TManager& manager, tlct::io::YuvPlanarFrame& dstFrame,
                                                   tlct::io::YuvPlanarFrame& dstFrameNormed) {
    const auto& bridge = manager.getBridge();
    const auto& arrange = manager.getArrange();

    const auto& extent = dstFrame.getExtent();
    auto srcSize = extent.getYSize();
    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
    }

    cv::Mat canvasNormed{srcSize, CV_8UC3};
    cv::Mat canvas{srcSize, CV_8UC3};

    const tlct::cfg::MITypes miTypes{arrange.isOutShift()};

    float maxPsize = std::numeric_limits<float>::lowest();
    float minPsize = std::numeric_limits<float>::max();
    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const float psize = bridge.getPatchsize(row, col);
            if (psize > maxPsize) maxPsize = psize;
            if (psize < minPsize) minPsize = psize;
        }
    }

    for (const int row : rgs::views::iota(0, arrange.getMIRows())) {
        for (const int col : rgs::views::iota(0, arrange.getMICols(row))) {
            const auto center = arrange.getMICenter(row, col);

            const float psize = bridge.getPatchsize(row, col);
            const cv::Scalar psizeNormedColor = cv::Scalar::all((psize - minPsize) / (maxPsize - minPsize) * 255.0);
            const cv::Scalar psizeColor = cv::Scalar::all(psize);
            cv::circle(canvasNormed, center, (int)arrange.getRadius(), psizeNormedColor, cv::FILLED, cv::LINE_AA);
            cv::circle(canvas, center, (int)arrange.getRadius(), psizeColor, cv::FILLED, cv::LINE_AA);

            const int miType = miTypes.getMIType(row, col);
            const cv::Scalar mitypeColor{
                255.0 * (int)(miType == 0),
                255.0 * (int)(miType == 1),
                255.0 * (int)(miType == 2),
            };
            cv::circle(canvasNormed, center, (int)arrange.getRadius(), mitypeColor, 1, cv::LINE_AA);
            cv::circle(canvas, center, (int)arrange.getRadius(), mitypeColor, 1, cv::LINE_AA);
        }
    }

    if (arrange.getDirection()) {
        cv::transpose(canvasNormed, canvasNormed);
        cv::transpose(canvas, canvas);
    }

    bgr2yuv(canvasNormed, dstFrameNormed);
    bgr2yuv(canvas, dstFrame);

    return {};
}

template <tlct::concepts::CManager TManager>
static std::expected<void, tlct::Error> paint(const tlct::CliConfig& cliCfg, const tlct::ConfigMap& map) noexcept {
    auto arrangeRes = TManager::TArrange::createWithCfgMap(map);
    if (!arrangeRes) return std::unexpected{std::move(arrangeRes.error())};
    auto& arrange = arrangeRes.value();

    cv::Size srcSize = arrange.getImgSize();
    const int upsample = cliCfg.convert.upsample;
    cv::Size dstSize = srcSize * upsample;
    arrange.upsample(upsample);

    auto managerRes = TManager::create(arrange, cliCfg.convert);
    if (!managerRes) return std::unexpected{std::move(managerRes.error())};
    auto& manager = managerRes.value();

    if (arrange.getDirection()) {
        std::swap(srcSize.width, srcSize.height);
        std::swap(dstSize.width, dstSize.height);
    }

    auto srcExtentRes = tlct::io::YuvPlanarExtent::createYuv420p8bit(srcSize.width, srcSize.height);
    if (!srcExtentRes) return std::unexpected{std::move(srcExtentRes.error())};
    auto srcExtent = srcExtentRes.value();

    auto dstExtentRes = tlct::io::YuvPlanarExtent::createYuv420p8bit(dstSize.width, dstSize.height);
    if (!dstExtentRes) return std::unexpected{std::move(dstExtentRes.error())};
    auto dstExtent = dstExtentRes.value();

    auto yuvReaderRes = tlct::io::YuvPlanarReader::create(cliCfg.path.src, srcExtent);
    if (!yuvReaderRes) return std::unexpected{std::move(yuvReaderRes.error())};
    auto& yuvReader = yuvReaderRes.value();

    const fs::path& dstdir = cliCfg.path.dst;
    fs::create_directories(dstdir);

    std::string filename = std::format("dbg-{}x{}.yuv", dstSize.width, dstSize.height);
    fs::path savetoPath = dstdir / filename;
    auto yuvWriterRes = tlct::io::YuvPlanarWriter::create(savetoPath);
    if (!yuvWriterRes) return std::unexpected{std::move(yuvWriterRes.error())};
    auto& yuvWriter = yuvWriterRes.value();

    auto skipRes = yuvReader.skip(cliCfg.range.begin);
    if (!skipRes) return std::unexpected{std::move(skipRes.error())};

    auto srcFrame = tlct::io::YuvPlanarFrame::create(srcExtent).value();
    auto dstFrameNormed = tlct::io::YuvPlanarFrame::create(dstExtent).value();
    auto dstFrame = tlct::io::YuvPlanarFrame::create(dstExtent).value();
    for ([[maybe_unused]] const int fid : rgs::views::iota(cliCfg.range.begin, cliCfg.range.end)) {
        auto readRes = yuvReader.readInto(srcFrame);
        if (!readRes) return std::unexpected{std::move(readRes.error())};

        auto updateRes = manager.update(srcFrame);
        if (!updateRes) return std::unexpected{std::move(updateRes.error())};

        auto paintRes = paintFrame(manager, dstFrame, dstFrameNormed);
        if (!paintRes) return std::unexpected{std::move(paintRes.error())};

        auto writeRes = yuvWriter.write(dstFrame);
        if (!writeRes) return std::unexpected{std::move(writeRes.error())};
        writeRes = yuvWriter.write(dstFrameNormed);
        if (!writeRes) return std::unexpected{std::move(writeRes.error())};

        cv::resize(srcFrame.getY(), dstFrame.getY(), {}, upsample, upsample, cv::INTER_CUBIC);
        cv::resize(srcFrame.getU(), dstFrame.getU(), {}, upsample, upsample, cv::INTER_CUBIC);
        cv::resize(srcFrame.getV(), dstFrame.getV(), {}, upsample, upsample, cv::INTER_CUBIC);
        writeRes = yuvWriter.write(dstFrame);
        if (!writeRes) return std::unexpected{std::move(writeRes.error())};
    }

    return {};
}

bool isMultiFocus(const tlct::ConfigMap& cfgMap) { return cfgMap.getOr<"NearFocalLenType">(-1) >= 0; }

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
        paint<tlct::cvt::TSPCMeth0Manager>, paint<tlct::cvt::RaytrixMeth0Manager>,
        paint<tlct::cvt::TSPCMeth1Manager>, paint<tlct::cvt::RaytrixMeth1Manager>,
        paint<tlct::cvt::TSPCDebugManager>, paint<tlct::cvt::RaytrixDebugManager>,
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

    const int pipeline = cliCfg.convert.method * 2 + (int)isMultiFocus(cfgMap);
    const auto& handler = handlers[pipeline];

    handler(cliCfg, cfgMap) | unwrap;
}
