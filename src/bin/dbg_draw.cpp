#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;
namespace rgs = std::ranges;

int main(int argc, char** argv)
{
    auto src = cv::imread("Boys.png");
    const auto config= cfg::tspc::CalibConfig::fromPath("Boys.xml");

    const auto size = config.getCentersSize();
    for (const int row : rgs::views::iota(0, size.height)) {
        for (const int col : rgs::views::iota(0, size.width)) {
            const auto center = config.getCenter(row, col);
            cv::circle(src, center, 35, {0, 0, 255}, 1, cv::LINE_AA);
        }
    }
    const auto center = config.getCenter(0, 1);
    cv::circle(src, center, 35, {255, 0, 0}, 1, cv::LINE_AA);

    cv::imwrite("dbg_center.png", src);
}
