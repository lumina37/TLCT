#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    cv::Mat src = cv::imread("Cars.png");
    cv::Mat resized_src;
    constexpr int factor = 4;
    cv::resize(src, resized_src, {}, factor, factor);
    src = resized_src;

    const auto config = cfg::CalibConfig::fromXMLPath("Cars.xml");
    const auto layout = cfg::Layout::fromConfigAndImgsize(config, src.size()).upsample(factor);

    const auto patchsizes = cvt::generatePatchsizes(src, layout);

    cv::imwrite("patchsizes.tiff", patchsizes);

    std::cout << patchsizes << std::endl;
}
