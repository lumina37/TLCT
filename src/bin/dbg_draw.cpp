#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    auto src = cv::imread("Cars.png");
    cfg::tspc::CalibConfig config{};
    config._setCenters("centers_cars.xml");

    const auto center = config.getCenter(0, 0);
    cv::circle(src, center, 35, {0, 0, 255}, -1, cv::LINE_AA);
    cv::imwrite("dbg_center.png", src);
}
