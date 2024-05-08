#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    const auto src = cv::imread("Boys.png");
    const auto config= cfg::tspc::CalibConfig::fromPath("Boys.xml");

    const auto patchsizes = cvt::tspc::generatePatchsizes(src, config);

    cv::imwrite("patchsizes.tiff", patchsizes);

    std::cout << patchsizes << std::endl;
}
