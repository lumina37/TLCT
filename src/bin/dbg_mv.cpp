#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main()
{
    cv::Mat src = cv::imread("Boys.png");
    constexpr int factor = 4;

    const auto config = cfg::tspc::CalibConfig::fromXMLPath("v2Boys.xml");
    const auto layout = cfg::tspc::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor);
    const cv::Mat resized_src = cfg::tspc::procImg(layout, src);

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::to_multiview(resized_src, layout, patchsizes, "./Boys", 5);
}
