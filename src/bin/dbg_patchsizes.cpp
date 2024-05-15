#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    const cv::Mat src = cv::imread("Cars.png");
    constexpr int factor = 4;

    const auto config = cfg::CalibConfig::fromXMLPath("Cars.xml");
    const auto layout = cfg::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor).transpose();
    const cv::Mat resized_src = cfg::procImg(layout, src);

    const auto patchsizes = cvt::estimatePatchsizes(layout, resized_src);

    cv::imwrite("patchsizes.tiff", patchsizes);

    std::cout << patchsizes << std::endl;
}