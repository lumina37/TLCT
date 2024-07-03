#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

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
            cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {0, 0, 255}, 1, cv::LINE_AA);
        }
    }
    for (const int col : rgs::views::iota(0, layout.getMICols(0))) {
        const auto center = layout.getMICenter(0, col);
        cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);
    }

    using NeighborIdx = tcvt::_hp::NeighborIdx_<tlct::cfg::tspc::Layout, 1>;
    const cv::Scalar base_color{0, 63, 63};
    auto neighbors = NeighborIdx::fromLayoutAndIndex(layout, {1, 1});
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpLeft()), tlct::_hp::iround(layout.getRadius()),
               base_color * 1, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpRight()), tlct::_hp::iround(layout.getRadius()),
               base_color * 2, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownLeft()), tlct::_hp::iround(layout.getRadius()),
               base_color * 3, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownRight()), tlct::_hp::iround(layout.getRadius()),
               base_color * 4, 2, cv::LINE_AA);

    neighbors = NeighborIdx::fromLayoutAndIndex(layout, {1, 4});
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpLeft()), tlct::_hp::iround(layout.getRadius()),
               base_color * 1, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpRight()), tlct::_hp::iround(layout.getRadius()),
               base_color * 2, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownLeft()), tlct::_hp::iround(layout.getRadius()),
               base_color * 3, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownRight()), tlct::_hp::iround(layout.getRadius()),
               base_color * 4, 2, cv::LINE_AA);

    cv::imwrite("dbg_center.png", resized_img);
}
