#include <ranges>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace rgs = std::ranges;

int main(int argc, char* argv[])
{
    const fs::path case_root{argv[1]};
    const auto cfgpath = case_root / "param.cfg";
    const auto cfgmap = tlct::ConfigMap::fromPath(cfgpath.string());
    cv::Mat resized_img;

    if (cfgmap.getPipelineType() == tlct::PipelineType::TLCT) {
        namespace tn = tlct::tspc;

        const auto param_cfg = tn::ParamConfig::fromConfigMap(cfgmap);
        const auto layout = tn::Layout::fromCalibAndSpecConfig(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg());

        const auto srcpath = case_root / "example.png";
        const cv::Mat src = cv::imread(srcpath.string());
        layout.processInto(src, resized_img);

        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const auto center = layout.getMICenter(row, col);
                resized_img.at<cv::Vec3b>(center) = {0, 0, 255};
                cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()), {255, 0, 0}, 1, cv::LINE_AA);
            }
        }
    } else {
        namespace tn = tlct::raytrix;

        const auto param_cfg = tn::ParamConfig::fromConfigMap(cfgmap);
        const auto layout = tn::Layout::fromCalibAndSpecConfig(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg());

        const auto srcpath = case_root / "example.png";
        const cv::Mat src = cv::imread(srcpath.string());
        layout.processInto(src, resized_img);

        for (const int row : rgs::views::iota(0, layout.getMIRows())) {
            for (const int col : rgs::views::iota(0, layout.getMICols(row))) {
                const auto center = layout.getMICenter(row, col);
                const auto mitype = layout.getMIType(row, col);
                const bool r = mitype == 0;
                const bool g = mitype == 1;
                const bool b = mitype == 2;
                cv::circle(resized_img, center, tlct::_hp::iround(layout.getRadius()),
                           {255.0 * b, 255.0 * g, 255.0 * r}, 1, cv::LINE_AA);
                resized_img.at<cv::Vec3b>(center) = {(uint8_t)(255 * b), (uint8_t)(255 * g), (uint8_t)(255 * r)};
            }
        }
    }

    const auto dstpath = case_root / "center.png";
    cv::imwrite(dstpath.string(), resized_img);
}
