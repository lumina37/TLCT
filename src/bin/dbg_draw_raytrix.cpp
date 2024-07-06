#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;
namespace tcvt = tlct::cvt::raytrix;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::cfg::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tcfg::ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    const auto layout =
        tcfg::Layout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg().getImgSize());

    const auto srcpath = common_cfg.fmtSrcPath(common_cfg.getRange().start);
    const cv::Mat src = cv::imread(srcpath.string());
    const cv::Mat resized_img = layout.procImg(src);

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

    using Neighbors = tcvt::_hp::Neighbors;
    const cv::Scalar base_color{0, 255.0 / 6, 255.0 / 6};
    auto neighbors = Neighbors::fromLayoutAndIndex(layout, {4, 3});
    for (const auto direction : tcvt::DIRECTIONS) {
        cv::circle(resized_img, neighbors.getNeighborPt(direction), tlct::_hp::iround(layout.getRadius()),
                   base_color * (int)direction, 2, cv::LINE_AA);
    }

    neighbors = tcvt::_hp::Neighbors::fromLayoutAndIndex(layout, {4, 8});
    for (const auto direction : tcvt::DIRECTIONS) {
        cv::circle(resized_img, neighbors.getNeighborPt(direction), tlct::_hp::iround(layout.getRadius()),
                   base_color * (int)direction, 2, cv::LINE_AA);
    }

    cv::imwrite("dbg_center.png", resized_img);
}
