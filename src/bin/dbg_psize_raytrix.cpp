#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::raytrix;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    auto state = tn::State::fromParamCfg(param_cfg);

    const auto srcpath = common_cfg.fmtSrcPath(1);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = state.estimatePatchsizes();

    std::cout << patchsizes << std::endl;
}