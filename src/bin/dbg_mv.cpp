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
    cv::Mat src = cv::imread(srcpath.string());

    auto state = tlct::cvt::State::fromLayoutAndViews(layout, param_cfg.getViews());
    state.feed(std::move(src));

    int img_cnt = 1;
    const fs::path dstdir{param_cfg.getDstPattern()};
    for (const auto& mv : state) {
        std::stringstream filename_s;
        filename_s << "image_" << std::setw(3) << std::setfill('0') << img_cnt << ".png";
        img_cnt++;
        const fs::path saveto_path = dstdir / filename_s.str();
        cv::imwrite(saveto_path.string(), mv);
    }
}
