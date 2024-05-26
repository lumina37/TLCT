#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

namespace tcfg = tlct::cfg::tspc;

int main()
{
    const cv::Mat src = cv::imread("Boys.png");
    constexpr int factor = 4;

    const auto config = tcfg::CalibConfig::fromXMLPath("v2Boys.xml");
    const auto layout = tcfg::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor);
    const cv::Mat resized_src = tcfg::Layout::procImg(layout, src);

    const auto patchsizes = tlct::cvt::estimatePatchsizes(layout, resized_src);

    cv::imwrite("patchsizes.tiff", patchsizes);

    std::cout << patchsizes << std::endl;
}
