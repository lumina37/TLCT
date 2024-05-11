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

    const auto config = cfg::tspc::CalibConfig::fromPath("Cars.xml");
    const auto layout = cfg::tspc::Layout::fromConfigAndImgsize(config, src.size()).upsample(factor);

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::_Lenslet_Rendering_zoom(src, layout, patchsizes, "./Cars", 1);
}
