#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tcfg = tlct::cfg::raytrix;
namespace tcvt = tlct::cvt::raytrix;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tcfg::ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    constexpr int upsample = 2;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);
    auto state = tcvt::State::fromLayoutAndViews(layout, common_cfg.getViews());

    const auto srcpath = common_cfg.fmtSrcPath(1);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = tcvt::estimatePatchsizes(state);

    std::cout << patchsizes << std::endl;
}