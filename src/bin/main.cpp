#include <array>
#include <filesystem>
#include <ranges>
#include <sstream>

#include <argparse/argparse.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

template <tlct::concepts::CState TState>
static inline void render(const tlct::ConfigMap& cfg_map)
{
    const auto param_cfg = TState::TParamConfig::fromConfigMap(cfg_map);
    const auto& generic_cfg = param_cfg.getGenericCfg();
    const auto& spec_cfg = param_cfg.getSpecificCfg();

    auto state = TState::fromParamCfg(param_cfg);

    const auto src_size = spec_cfg.getImgSize();
    const auto output_size = state.getOutputSize();

    using TYuvReader = tlct::io::YuvReader_<typename TState::TFrame>;
    using TYuvWriter = tlct::io::YuvWriter_<typename TState::TFrame>;
    auto yuv_reader = TYuvReader::fromPath(generic_cfg.getSrcPath(), src_size.width, src_size.height);

    const fs::path& dstdir = generic_cfg.getDstPath();
    fs::create_directories(dstdir);
    std::vector<TYuvWriter> yuv_writers;
    const int total_writers = generic_cfg.getViews() * generic_cfg.getViews();
    yuv_writers.reserve(total_writers);
    for (const auto i : rgs::views::iota(0, total_writers)) {
        std::stringstream filename_s;
        filename_s << 'v' << std::setw(3) << std::setfill('0') << i << '-' << output_size.width << 'x'
                   << output_size.height << ".yuv";
        fs::path saveto_path = dstdir / filename_s.str();
        yuv_writers.emplace_back(TYuvWriter::fromPath(saveto_path));
    }

    const cv::Range range = generic_cfg.getRange();
    yuv_reader.skip(range.start);

    auto frame = typename TState::TFrame{src_size};
    auto mv = typename TState::TFrame{output_size};
    for (int fid = range.start; fid < range.end; fid++) {
        yuv_reader.read_into(frame);
        state.update(frame);

        int view = 0;
        for (const int view_row : rgs::views::iota(0, generic_cfg.getViews())) {
            for (const int view_col : rgs::views::iota(0, generic_cfg.getViews())) {
                auto& yuv_writer = yuv_writers[view];
                state.renderInto(mv, view_row, view_col);
                yuv_writer.write(mv);
                view++;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("TLCT", "v" tlct_VERSION, argparse::default_arguments::all);
    program.add_argument("param_file_path").help("the parameter file path").required();
    program.add_epilog(TLCT_COMPILE_INFO);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    const auto& param_file_path = program.get<std::string>("param_file_path");
    const auto cfg_map = tlct::ConfigMap::fromPath(param_file_path);

    constexpr std::array<void (*)(const tlct::ConfigMap&), tlct::PIPELINE_COUNT> handlers{
        render<tlct::raytrix::StateYuv420>,
        render<tlct::tspc::StateYuv420>,
    };
    const auto& handler = handlers[cfg_map.getPipelineType()];

    try {
        handler(cfg_map);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::exit(2);
    }
}
