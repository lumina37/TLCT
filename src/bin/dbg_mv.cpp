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
    const auto layout = cfg::Layout::fromCfgAndImgsize(config, src.size()).upsample(factor);

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::to_multiview(src, layout, patchsizes, "./Cars", 5);
}
