#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    const auto src = cv::imread("Cars.png");
    cfg::tspc::CalibConfig config{};
    config._setCenters("centers_cars.xml");

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::_Lenslet_Rendering_zoom(src, config, patchsizes, "./Cars", 5);
}
