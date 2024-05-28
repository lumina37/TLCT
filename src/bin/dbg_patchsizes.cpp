#include <filesystem>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace tcfg = tlct::cfg;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
    const auto common_cfg = tlct::cfg::CommonParamConfig::fromPath(argv[1]);
    const auto param_cfg = tcfg::tspc::ParamConfig::fromCommonCfg(common_cfg);

    constexpr int upsample = 2;
    const auto layout =
        tcfg::tspc::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);

    const fs::path srcpath{param_cfg.getSrcPattern()};

    const cv::Mat& src = cv::imread(srcpath.string());
    const cv::Mat resized_src = tcfg::tspc::Layout::procImg(layout, src);

    const cv::Mat& patchsizes = tlct::cvt::estimatePatchsizes(layout, resized_src);

    const fs::path dstdir{param_cfg.getDstPattern()};
    fs::create_directories(dstdir);
    const auto dstpath = dstdir / "patchsizes.tiff";

    cv::imwrite(dstpath.string(), patchsizes);

    std::cout << patchsizes << std::endl;
}
