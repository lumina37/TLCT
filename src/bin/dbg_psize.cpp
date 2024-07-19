#include <iostream>

#include <opencv2/core.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::tspc;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
    const auto& generic_cfg = param_cfg.getGenericCfg();

    auto state = tn::State::fromParamCfg(param_cfg);

    const auto srcpath = generic_cfg.fmtSrcPath(generic_cfg.getRange().start);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = state.estimatePatchsizes();

    std::cout << patchsizes << std::endl;
}