#include <filesystem>
#include <sstream>

#include <argparse/argparse.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::tspc;

template <typename TState>
    requires tlct::cvt::concepts::CState<TState>
static inline void render(const tlct::cfg::ConfigMap& cfg_map)
{
    const auto param_cfg = TState::TParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    auto state = TState::fromParamCfg(param_cfg);

    const cv::Range range = common_cfg.getRange();
    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = common_cfg.fmtSrcPath(i);
        state.feed(cv::imread(srcpath.string()));

        int img_cnt = 1;
        const auto dstdir = common_cfg.fmtDstPath(i);
        fs::create_directories(dstdir);

        for (const auto& mv : state) {
            std::stringstream filename_s;
            filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
            const fs::path saveto_path = dstdir / filename_s.str();
            cv::imwrite(saveto_path.string(), mv);
            img_cnt++;
        }
    }
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("TLCT", TLCT_VERSION, argparse::default_arguments::all);
    program.add_argument("param_file_path").help("the RLC parameter file path").required();

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    const auto& param_file_path = program.get<std::string>("param_file_path");

    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(param_file_path);

    if (cfg_map.getPipelineType() == tlct::cfg::PipelineType::RLC) {
        render<tlct::raytrix::State>(cfg_map);
    } else {
        render<tlct::tspc::State>(cfg_map);
    }
}
