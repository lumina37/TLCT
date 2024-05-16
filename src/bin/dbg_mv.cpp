#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main()
{
    cv::Mat src = cv::imread("Cars.png");
    constexpr int factor = 4;

    const auto config = cfg::CalibConfig::fromXMLPath("Cars.xml");
    const auto layout = cfg::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor);
    const cv::Mat resized_src = cfg::procImg(layout, src);

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::to_multiview(resized_src, layout, patchsizes, "./Cars", 3);
}
