#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;
namespace rgs = std::ranges;

int main(int argc, char** argv)
{
    auto src = cv::imread("Cars.png");
    const auto config = cfg::CalibConfig::fromXMLPath("Cars.xml");
    const auto layout = cfg::Layout::fromConfigAndImgsize(config, src.size());

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols())) {
            const auto center = layout.getMICenter(row, col);
            cv::circle(src, center, iround(layout.getRadius()), {0, 0, 255}, 1, cv::LINE_AA);
        }
    }
    const auto center = layout.getMICenter(0, 1);
    cv::circle(src, center, iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);

    cv::imwrite("dbg_center.png", src);
}
