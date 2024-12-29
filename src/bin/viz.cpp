#include <ranges>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

static inline cv::Mat convertToRGB(const tlct::io::Yuv420Frame& frame)
{
    cv::Mat u, v, yuv, rgb;
    const cv::Size& ysize = frame.getY().size();
    cv::resize(frame.getU(), u, ysize, 0., 0., cv::INTER_CUBIC);
    cv::resize(frame.getV(), v, ysize, 0., 0., cv::INTER_CUBIC);
    std::vector<cv::Mat> yuv_channels = {frame.getY(), u, v};
    cv::merge(yuv_channels, yuv);
    cv::cvtColor(yuv, rgb, cv::COLOR_YUV2BGR);
    return rgb;
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser parser{"Viz", "v" tlct_VERSION, argparse::default_arguments::all};

    parser.add_argument("calib_file").help("path of the `calib.cfg`").required();
    parser.add_argument("-i", "--src").help("input yuv420 planar file").required();
    parser.add_argument("-o", "--dst")
        .help("output directory, and the output file name is like 'v000-1920x1080.yuv' (v{view}-{wdt}x{hgt}.yuv)")
        .required();

    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    const auto& calib_file_path = parser.get<std::string>("calib_file");
    const auto& cfg_map = tlct::ConfigMap::fromPath(calib_file_path);
    const int pipeline = ((cfg_map.get_or<"IsKepler">(0) << 1) | cfg_map.get_or<"IsMultiFocus">(0)) - 1;

    cv::Mat canvas;

    if (pipeline == 0) {
        namespace tn = tlct::raytrix;

        const auto& layout = tn::Layout::fromCfgMap(cfg_map);
        const auto& mitypes = tlct::cfg::MITypes(layout.isOutShift());
        const cv::Size& raw_size = layout.getRawImgSize();
        auto yuv_reader =
            tlct::io::Yuv420Reader::fromPath(parser.get<std::string>("--src"), raw_size.width, raw_size.height);

        const auto& frame = yuv_reader.read();
        cv::Mat rgb = convertToRGB(frame);
        layout.processInto(rgb, canvas);

        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const auto center = layout.getMICenter(row, col);
                const auto mitype = mitypes.getMIType(row, col);
                const bool r = mitype == 0;
                const bool g = mitype == 1;
                const bool b = mitype == 2;
                cv::circle(canvas, center, tlct::_hp::iround(layout.getRadius()), {255.0 * b, 255.0 * g, 255.0 * r}, 1,
                           cv::LINE_AA);
                canvas.at<cv::Vec3b>(center) = {(uint8_t)(255 * b), (uint8_t)(255 * g), (uint8_t)(255 * r)};
            }
        }
    } else {
        namespace tn = tlct::tspc;

        const auto& layout = tn::Layout::fromCfgMap(cfg_map);
        const auto& raw_size = layout.getRawImgSize();
        auto yuv_reader =
            tlct::io::Yuv420Reader::fromPath(parser.get<std::string>("--src"), raw_size.width, raw_size.height);

        const auto& frame = yuv_reader.read();
        cv::Mat rgb = convertToRGB(frame);
        layout.processInto(rgb, canvas);

        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const cv::Point2d& center = layout.getMICenter(row, col);
                canvas.at<cv::Vec3b>(center) = {0, 0, 255};
                cv::circle(canvas, center, tlct::_hp::iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);
            }
        }
    }

    const fs::path dstpath{parser.get<std::string>("--dst")};
    cv::imwrite(dstpath.string(), canvas);
}
