#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::tspc;
namespace tcvt = tlct::cvt::tspc;

int main()
{
    const cv::Mat src = cv::imread("Boys/src/Image000.bmp");
    const auto config = tcfg::CalibConfig::fromXMLPath("Boys/calib.xml");
    const auto layout = tcfg::Layout::fromCfgAndImgsize(config, src.size());
    const cv::Mat resized_img = tcfg::Layout::procImg(layout, src);

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

    auto neighbors = tcvt::_hp::NeighborIdx_::fromLayoutAndIndex(layout, {1, 1});
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpLeft()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 1, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpRight()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 2, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownLeft()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 3, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownRight()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 4, 0}, 2, cv::LINE_AA);

    neighbors = tcvt::_hp::NeighborIdx_::fromLayoutAndIndex(layout, {1, 4});
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpLeft()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 1, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getUpRight()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 2, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownLeft()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 3, 0}, 2, cv::LINE_AA);
    cv::circle(resized_img, layout.getMICenter(neighbors.getDownRight()), tlct::_hp::iround(layout.getRadius()),
               {0, 63 * 4, 0}, 2, cv::LINE_AA);

    cv::imwrite("dbg_center.png", resized_img);
}
