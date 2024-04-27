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

    cv::Mat patchsizes;
    cvt::_Patch_Size_Cal(src, patchsizes, config);

    cv::imwrite("patchsizes.tiff",patchsizes);
}
