#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace rgs = std::ranges;
namespace tn = tlct::raytrix;

int main(int argc, char* argv[])
{
    const auto cfg_map = tlct::ConfigMap::fromPath(argv[1]);
    const auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
    const auto& common_cfg = param_cfg.getGenericCfg();

    const auto layout = tn::Layout::fromParamConfig(param_cfg);

    const auto srcpath = common_cfg.fmtSrcPath(common_cfg.getRange().start);
    const cv::Mat src = cv::imread(srcpath.string());
    const cv::Mat resized_img = layout.procImg(src);

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto center = layout.getMICenter(row, col);
            const auto mitype = layout.getMIType(row, col);
            const bool r = mitype == 0;
            const bool g = mitype == 1;
            const bool b = mitype == 2;
            cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255.0 * b, 255.0 * g, 255.0 * r}, 1,
                       cv::LINE_AA);
        }
    }

    const cv::Scalar base_color{0, 255.0 / 6, 255.0 / 6};
    auto neighbors = tn::Neighbors::fromLayoutAndIndex(layout, {4, 3});
    for (const auto direction : tlct::DIRECTIONS) {
        cv::circle(resized_img, neighbors.getNeighborPt(direction), tlct::_hp::iround(layout.getRadius()),
                   base_color * (int)direction, 2, cv::LINE_AA);
    }

    neighbors = tn::Neighbors::fromLayoutAndIndex(layout, {4, 8});
    for (const auto direction : tlct::DIRECTIONS) {
        cv::circle(resized_img, neighbors.getNeighborPt(direction), tlct::_hp::iround(layout.getRadius()),
                   base_color * (int)direction, 2, cv::LINE_AA);
    }

    cv::imwrite("dbg_center.png", resized_img);
}
