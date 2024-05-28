#include <filesystem>

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

    const fs::path dstdir{param_cfg.getDstPattern()};
    const auto patchsize_path = dstdir / "patchsizes.tiff";
    const cv::Mat& patchsizes = cv::imread(patchsize_path.string(), cv::IMREAD_UNCHANGED);

    tlct::cvt::to_multiview(resized_src, layout, patchsizes, dstdir, param_cfg.getViews());
}
