#include <filesystem>
#include <sstream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;
namespace fs = std::filesystem;

using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

int main(int argc, char* argv[])
{
    const auto common_cfg = tlct::cfg::CommonParamConfig::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromCommonCfg(common_cfg);

    constexpr int upsample = 2;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);
    auto state = tcvt::State::fromLayoutAndViews(layout, param_cfg.getViews());

    const cv::Range range = param_cfg.getRange();
    if (range.size() > 1) {
        // For a better patch size estimation
        const auto initpath = ParamConfig::fmtSrcPath(param_cfg, range.start);
        state.feed(cv::imread(initpath.string()));
    }

    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = ParamConfig::fmtSrcPath(param_cfg, i);
        state.feed(cv::imread(srcpath.string()));

        int img_cnt = 1;
        const auto dstdir = ParamConfig::fmtDstPath(param_cfg, i);
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
