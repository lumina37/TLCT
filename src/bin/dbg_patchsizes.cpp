#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main()
{
    const cv::Mat src = cv::imread("Boys.png");
    constexpr int factor = 4;

    const auto config = cfg::tspc::CalibConfig::fromXMLPath("v2Boys.xml");
    const auto layout = cfg::tspc::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor);
    const cv::Mat resized_src = cfg::tspc::procImg(layout, src);

    const auto patchsizes = cvt::estimatePatchsizes(layout, resized_src);

    cv::imwrite("patchsizes.tiff", patchsizes);

    std::cout << patchsizes << std::endl;
}
