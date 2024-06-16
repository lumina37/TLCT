#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

int main(int argc, char* argv[])
{
    using ParamConfig = tcfg::ParamConfig<tcfg::CalibConfig>;

    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getCommonCfg();

    const auto layout = tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getImgSize());

    const auto srcpath = tlct::cfg::CommonParamConfig::fmtSrcPath(common_cfg, 1);
    const cv::Mat src = cv::imread(srcpath.string());
    const cv::Mat resized_img = tcfg::Layout::procImg(layout, src);

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto center = layout.getMICenter(row, col);
            const bool r = layout.getMIType(row, col) == 0;
            const bool g = layout.getMIType(row, col) == 1;
            const bool b = layout.getMIType(row, col) == 2;
            cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255.0 * b, 255.0 * g, 255.0 * r}, 1,
                       cv::LINE_AA);
        }
    }

    cv::imwrite("dbg_center.png", resized_img);
}