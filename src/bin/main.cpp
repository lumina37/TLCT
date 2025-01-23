#include <array>
#include <filesystem>
#include <ranges>
#include <sstream>

#include <argparse/argparse.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CManager TManager>
static inline void render(const argparse::ArgumentParser& parser, const tlct::ConfigMap& map)
{
    const auto& cli_cfg = tlct::CliConfig::fromParser(parser);
    auto arrange = TManager::TArrange::fromCfgMap(map);
    cv::Size src_size = arrange.getImgSize();
    arrange.upsample(cli_cfg.convert.upsample);

    auto manager = TManager::fromConfigs(arrange, cli_cfg.convert);

    cv::Size output_size = manager.getOutputSize();
    if (arrange.getDirection()) {
        std::swap(src_size.width, src_size.height);
        std::swap(output_size.width, output_size.height);
    }

    using TYuvReader = tlct::io::YuvReader_<typename TManager::TFrame>;
    using TYuvWriter = tlct::io::YuvWriter_<typename TManager::TFrame>;
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

    auto src_frame = typename TManager::TFrame{src_size};
    auto mv_frame = typename TManager::TFrame{output_size};
    for (int fid = cli_cfg.range.begin; fid < cli_cfg.range.end; fid++) {
        yuv_reader.read_into(src_frame);
        manager.update(src_frame);

        int view = 0;
        for (const int view_row : rgs::views::iota(0, cli_cfg.convert.views)) {
            for (const int view_col : rgs::views::iota(0, cli_cfg.convert.views)) {
                auto& yuv_writer = yuv_writers[view];
                manager.renderInto(mv_frame, view_row, view_col);
                yuv_writer.write(mv_frame);
                view++;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    auto parser = tlct::makeUniqArgParser();

    try {
        parser->parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << *parser;
        std::exit(1);
    }

    constexpr std::array<void (*)(const argparse::ArgumentParser&, const tlct::ConfigMap&), 2> handlers{
        render<tlct::raytrix::ManagerYuv420>,
        render<tlct::tspc::ManagerYuv420>,
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
