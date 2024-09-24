#include <iostream>

#include <argparse/argparse.hpp>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;

template <typename TState>
    requires tlct::cvt::concepts::CState<TState>
static inline void estimate(const tlct::cfg::ConfigMap& cfg_map)
{
    const auto param_cfg = TState::TParamConfig::fromConfigMap(cfg_map);
    const auto& generic_cfg = param_cfg.getGenericCfg();

    auto state = TState::fromParamCfg(param_cfg);

    const auto srcpath = generic_cfg.fmtSrcPath(generic_cfg.getRange().start);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = state.estimatePatchsizes();

    std::cout << patchsizes << std::endl;
}

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("DebugPsize", TLCT_GIT_TAG, argparse::default_arguments::all);
    program.add_argument("param_file_path").help("the parameter file path").required();

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
        estimate<tlct::raytrix::State>(cfg_map);
    } else {
        estimate<tlct::tspc::State>(cfg_map);
    }
}
