#include <filesystem>
#include <sstream>

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

    auto state = tcvt::State::fromParamCfg(param_cfg);
    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg().getImgSize());
    //    auto inspector = tcvt::Inspector::fromGenericCfg(param_cfg.getGenericCfg());
    //    inspector.setEnableIf([](cv::Point index) { return index == cv::Point{10, 11}; });
    //    state.setInspector(std::move(inspector));

    const cv::Range range = common_cfg.getRange();
    for (int i = range.start; i <= range.end; i++) {
        const auto srcpath = common_cfg.fmtSrcPath(i);
        state.feed(cv::imread(srcpath.string()));

        int img_cnt = 1;
        const auto dstdir = common_cfg.fmtDstPath(i);
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
