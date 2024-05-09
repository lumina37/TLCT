#include <opencv2/imgcodecs.hpp>

#include "tlct/convert/tspc.hpp"

using namespace tlct;

int main(int argc, char** argv)
{
    const auto src = cv::imread("Cars.png");
    const auto config= cfg::tspc::CalibConfig::fromPath("Cars.xml");

    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    cvt::tspc::_Lenslet_Rendering_zoom(src, config, patchsizes, "./Cars", 1);
}
