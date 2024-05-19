#pragma once

#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"

namespace tlct::cfg::tspc::inline v2 {

class Layout;

class CalibConfig
{
public:
    friend class Layout;

    TLCT_API CalibConfig() noexcept : left_top_(), right_top_(), left_bottom_(), diameter_(0.0), rotation_(0.0) {};
    TLCT_API CalibConfig& operator=(const CalibConfig& cfg) noexcept = default;
    TLCT_API CalibConfig(const CalibConfig& cfg) noexcept = default;
    TLCT_API CalibConfig& operator=(CalibConfig&& cfg) noexcept = default;
    TLCT_API CalibConfig(CalibConfig&& cfg) noexcept = default;
    TLCT_API CalibConfig(const cv::Point2d left_top, const cv::Point2d right_top, const cv::Point2d left_bottom,
                         double diameter, double rotation) noexcept
        : left_top_(left_top), right_top_(right_top), left_bottom_(left_bottom), diameter_(diameter),
          rotation_(rotation) {};

    [[nodiscard]] TLCT_API static CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static CalibConfig fromXMLPath(const char* path);

private:
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_bottom_;
    double diameter_;
    double rotation_;
};

inline CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{

    const auto data_node = doc.child("TSPCCalibData");
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    const auto centers_node = data_node.child("centers");

    const auto left_top_node = centers_node.child("left_top");
    const double left_top_x = left_top_node.child("x").text().as_double();
    const double left_top_y = left_top_node.child("y").text().as_double();
    const cv::Point2d left_top{left_top_x, left_top_y};

    const auto right_top_node = centers_node.child("right_top");
    const double right_top_x = right_top_node.child("x").text().as_double();
    const double right_top_y = right_top_node.child("y").text().as_double();
    const cv::Point2d right_top{right_top_x, right_top_y};

    const auto left_bottom_node = centers_node.child("left_bottom");
    const double left_bottom_x = left_bottom_node.child("x").text().as_double();
    const double left_bottom_y = left_bottom_node.child("y").text().as_double();
    const cv::Point2d left_bottom{left_bottom_x, left_bottom_y};

    return {left_top, right_top, left_bottom, diameter, rotation};
}

inline CalibConfig CalibConfig::fromXMLPath(const char* path)
{
    pugi::xml_document doc;
    const auto ret = doc.load_file(path, pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return {};
    }
    return CalibConfig::fromXMLDoc(doc);
}

} // namespace tlct::cfg::_experiment
