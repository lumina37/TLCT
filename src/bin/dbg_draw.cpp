#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace tcfg = tlct::cfg::tspc;
namespace rgs = std::ranges;

int main()
{
    const cv::Mat src = cv::imread("NagoyaFujita.png");
    const auto config = tcfg::CalibConfig::fromXMLPath("NagoyaFujita.xml");
    const auto layout = tcfg::Layout::fromCfgAndImgsize(config, src.size());
    const cv::Mat resized_img = tcfg::Layout::procImg(layout, src);

    for (const int row : rgs::views::iota(0, layout.getMIRows())) {
        for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
            const auto center = layout.getMICenter(row, col);
            cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {0, 0, 255}, 1, cv::LINE_AA);
        }
    }
    for (const int col : rgs::views::iota(0, layout.getMICols(0))) {
        const auto center = layout.getMICenter(0, col);
        cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);
    }

    cv::Mat transposed_src;
    cv::transpose(resized_img, transposed_src);
    cv::imwrite("dbg_center.png", transposed_src);
}
