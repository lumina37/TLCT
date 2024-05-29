#include <filesystem>
#include <sstream>

#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;
namespace fs = std::filesystem;

using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

int main(int argc, char* argv[])
{
    const auto common_cfg = tlct::cfg::CommonParamConfig::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromCommonCfg(common_cfg);

    constexpr int upsample = 2;
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize()).upsample(upsample);
    auto state = tlct::cvt::State::fromLayoutAndViews(layout, param_cfg.getViews());
    const cv::Range range = param_cfg.getRange();

    const auto initpath = ParamConfig::fmtSrcPath(param_cfg, range.start);
    cv::Mat init = cv::imread(initpath.string());
    state.feed(std::move(init));

    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = ParamConfig::fmtSrcPath(param_cfg, i);

        cv::Mat src = cv::imread(srcpath.string());
        state.feed(std::move(src));

        int img_cnt = 1;
        for (const auto& mv : state) {
            std::stringstream filename_s;
            filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
            img_cnt++;

            const auto dstdir = ParamConfig::fmtDstPath(param_cfg, i);
            fs::create_directories(dstdir);
            const fs::path saveto_path = dstdir / filename_s.str();

            cv::imwrite(saveto_path.string(), mv);
        }
    }
}
