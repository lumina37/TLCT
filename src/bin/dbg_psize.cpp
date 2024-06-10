#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

int main(int argc, char* argv[])
{
    using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

    const auto common_cfg = tlct::cfg::CommonParamConfig::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromCommonCfg(common_cfg);

    constexpr int upsample = 1;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);
    auto state = tcvt::State::fromLayoutAndViews(layout, param_cfg.getViews());

    const auto srcpath = ParamConfig::fmtSrcPath(param_cfg, 1);

    const cv::Mat& src = cv::imread(srcpath.string());
    state.feed(src);

    const cv::Mat& patchsizes = tcvt::estimatePatchsizes(state);

    std::cout << patchsizes << std::endl;
}