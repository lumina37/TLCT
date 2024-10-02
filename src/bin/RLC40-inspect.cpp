#include <filesystem>
#include <ranges>
#include <sstream>

#include <argparse/argparse.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;
namespace tn = tlct::tspc;

template <typename TState>
    requires tlct::cvt::concepts::CState<TState>
static inline void render(const tlct::cfg::ConfigMap& cfg_map)
{
    const auto param_cfg = TState::TParamConfig::fromConfigMap(cfg_map);
    const auto& generic_cfg = param_cfg.getGenericCfg();

    auto state = TState::fromParamCfg(param_cfg);
    auto inspector = tlct::Inspector::fromGenericCfg(generic_cfg);
    inspector.setEnableIf([](cv::Point idx) { return idx.x == 35 && idx.y == 35; });
    state.setInspector(std::move(inspector));

    const cv::Range range = generic_cfg.getRange();
    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = generic_cfg.fmtSrcPath(i);
        state.feed(cv::imread(srcpath.string()));

        const auto dstdir = generic_cfg.fmtDstPath(i);
        fs::create_directories(dstdir);

        int img_cnt = 1;
        cv::Mat mv;
        for (const int view_row : rgs::views::iota(0, generic_cfg.getViews())) {
            for (const int view_col : rgs::views::iota(0, generic_cfg.getViews())) {
                std::stringstream filename_s;
                filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
                const fs::path saveto_path = dstdir / filename_s.str();
                state.renderInto(mv, view_row, view_col);
                cv::imwrite(saveto_path.string(), mv);
                img_cnt++;
            }
        }
    }
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("TLCT", TLCT_GIT_TAG, argparse::default_arguments::all);
    program.add_argument("param_file_path").help("the parameter file path").required();
    program.add_description(TLCT_COMPILE_INFO);

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
