#include <array>
#include <filesystem>
#include <ranges>
#include <sstream>

#include <argparse/argparse.hpp>
#include <toml++/toml.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CState TState>
static inline void render(const argparse::ArgumentParser& parser)
{
    const auto& common_cfg = tlct::CommonConfig::fromParser(parser);

    const auto& calib_file_path = parser.get<std::string>("calib_file");
    auto calib_table = toml::parse_file(calib_file_path);
    const auto layout = TState::TLayout::fromToml(calib_table).upsample(common_cfg.convert.upsample);

    auto state = TState::fromConfigs(layout, common_cfg.convert);

    const auto src_size = layout.getRawImgSize();
    const auto output_size = state.getOutputSize();

    using TYuvReader = tlct::io::YuvReader_<typename TState::TFrame>;
    using TYuvWriter = tlct::io::YuvWriter_<typename TState::TFrame>;
    auto yuv_reader = TYuvReader::fromPath(common_cfg.path.src, src_size.width, src_size.height);

    const fs::path& dstdir = common_cfg.path.dst;
    fs::create_directories(dstdir);
    std::vector<TYuvWriter> yuv_writers;
    const int total_writers = common_cfg.convert.views * common_cfg.convert.views;
    yuv_writers.reserve(total_writers);
    for (const auto i : rgs::views::iota(0, total_writers)) {
        std::stringstream filename_s;
        filename_s << 'v' << std::setw(3) << std::setfill('0') << i << '-' << output_size.width << 'x'
                   << output_size.height << ".yuv";
        fs::path saveto_path = dstdir / filename_s.str();
        yuv_writers.emplace_back(TYuvWriter::fromPath(saveto_path));
    }

    yuv_reader.skip(common_cfg.range.begin);

    auto frame = typename TState::TFrame{src_size};
    auto mv = typename TState::TFrame{output_size};
    for (int fid = common_cfg.range.begin; fid < common_cfg.range.end; fid++) {
        yuv_reader.read_into(frame);
        state.update(frame);

        int view = 0;
        for (const int view_row : rgs::views::iota(0, common_cfg.convert.views)) {
            for (const int view_col : rgs::views::iota(0, common_cfg.convert.views)) {
                auto& yuv_writer = yuv_writers[view];
                state.renderInto(mv, view_row, view_col);
                yuv_writer.write(mv);
                view++;
            }
        }
    }
}

static inline int getPipelineIdx(const argparse::ArgumentParser& parser)
{
    const int is_kepler = parser.get<bool>("--isKepler");
    const int is_mf = parser.get<bool>("--multiFocus");
    const int idx = ((is_kepler << 1) | is_mf) - 1;
    return idx;
}

int main(int argc, char* argv[])
{
    auto parser = tlct::newParser();

    try {
        parser->parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << *parser;
        std::exit(1);
    }

    constexpr std::array<void (*)(const argparse::ArgumentParser&), 2> handlers{
        render<tlct::raytrix::StateYuv420>,
        render<tlct::tspc::StateYuv420>,
    };
    const int pipeline = getPipelineIdx(*parser);

    try {
        const auto& handler = handlers[pipeline];
        handler(*parser);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::exit(2);
    }
}
