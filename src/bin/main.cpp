#include <array>
#include <filesystem>
#include <ranges>
#include <sstream>

#include <argparse/argparse.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CState TState>
static inline void render(const argparse::ArgumentParser& parser, const tlct::ConfigMap& map)
{
    const auto& cli_cfg = tlct::CliConfig::fromParser(parser);
    const auto layout = TState::TLayout::fromCfgMap(map).upsample(cli_cfg.convert.upsample);

    auto state = TState::fromConfigs(layout, cli_cfg.convert);

    const cv::Size& src_size = layout.getRawImgSize();
    const cv::Size& output_size = state.getOutputSize();

    using TYuvReader = tlct::io::YuvReader_<typename TState::TFrame>;
    using TYuvWriter = tlct::io::YuvWriter_<typename TState::TFrame>;
    auto yuv_reader = TYuvReader::fromPath(cli_cfg.path.src, src_size.width, src_size.height);

    const fs::path& dstdir = cli_cfg.path.dst;
    fs::create_directories(dstdir);
    std::vector<TYuvWriter> yuv_writers;
    const int total_writers = cli_cfg.convert.views * cli_cfg.convert.views;
    yuv_writers.reserve(total_writers);
    for (const int i : rgs::views::iota(0, total_writers)) {
        std::stringstream filename_s;
        filename_s << 'v' << std::setw(3) << std::setfill('0') << i << '-' << output_size.width << 'x'
                   << output_size.height << ".yuv";
        fs::path saveto_path = dstdir / filename_s.str();
        yuv_writers.emplace_back(TYuvWriter::fromPath(saveto_path));
    }

    yuv_reader.skip(cli_cfg.range.begin);

    auto src_frame = typename TState::TFrame{src_size};
    auto mv_frame = typename TState::TFrame{output_size};
    for (int fid = cli_cfg.range.begin; fid < cli_cfg.range.end; fid++) {
        yuv_reader.read_into(src_frame);
        state.update(src_frame);

        int view = 0;
        for (const int view_row : rgs::views::iota(0, cli_cfg.convert.views)) {
            for (const int view_col : rgs::views::iota(0, cli_cfg.convert.views)) {
                auto& yuv_writer = yuv_writers[view];
                state.renderInto(mv_frame, view_row, view_col);
                yuv_writer.write(mv_frame);
                view++;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    auto parser = tlct::makeParser();

    try {
        parser->parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << *parser;
        std::exit(1);
    }

    constexpr std::array<void (*)(const argparse::ArgumentParser&, const tlct::ConfigMap&), 2> handlers{
        render<tlct::raytrix::StateYuv420>,
        render<tlct::tspc::StateYuv420>,
    };

    try {
        const auto& calib_file_path = parser->get<std::string>("calib_file");
        const auto& cfg_map = tlct::ConfigMap::fromPath(calib_file_path);
        const int pipeline = ((cfg_map.get_or<"IsKepler">(0) << 1) | cfg_map.get_or<"IsMultiFocus">(0)) - 1;
        const auto& handler = handlers[pipeline];
        handler(*parser, cfg_map);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::exit(2);
    }
}
