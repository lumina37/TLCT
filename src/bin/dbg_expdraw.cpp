#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

using namespace tlct::cfg::tspc::v2;
namespace rgs = std::ranges;

int main()
{
    const cv::Mat src = cv::imread("Cars.png");
    const auto config = CalibConfig::fromXMLPath("v2Cars.xml");
    const auto layout = Layout::fromCfgAndImgsize(config, src.size());
    const cv::Mat resized_img = procImg(layout, src);

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols())) {
            const auto center = layout.getMICenter(row, col);
            cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {0, 0, 255}, 1, cv::LINE_AA);
        }
    }
    for (const int col : rgs::views::iota(0, layout.getMICols())) {
        const auto center = layout.getMICenter(0, col);
        cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);
    }

    cv::imwrite("dbg_centerexp.png", resized_img);
}
