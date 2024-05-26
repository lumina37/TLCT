#pragma once

#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"

namespace tlct::cfg::tspc {

class Layout;

class CalibConfig
{
public:
    friend class Layout;

    TLCT_API CalibConfig() noexcept
        : left_top_(), right_top_(), left_bottom_(), rows_(), cols_(), diameter_(), rotation_() {};
    TLCT_API CalibConfig& operator=(const CalibConfig& cfg) noexcept = default;
    TLCT_API CalibConfig(const CalibConfig& cfg) noexcept = default;
    TLCT_API CalibConfig& operator=(CalibConfig&& cfg) noexcept = default;
    TLCT_API CalibConfig(CalibConfig&& cfg) noexcept = default;
    TLCT_API CalibConfig(const cv::Point2d left_top, const cv::Point2d right_top, const cv::Point2d left_bottom,
                         int rows, int cols, double diameter, double rotation) noexcept
        : left_top_(left_top), right_top_(right_top), left_bottom_(left_bottom), rows_(rows), cols_(cols),
          diameter_(diameter), rotation_(rotation) {};

    [[nodiscard]] TLCT_API static CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static CalibConfig fromXMLPath(const char* path);

private:
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_bottom_;
    int rows_;
    int cols_;
    double diameter_;
    double rotation_;
};

static_assert(concepts::CCalibConfig<CalibConfig>);

inline CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{

    const auto data_node = doc.child("TSPCCalibData");
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    const auto centers_node = data_node.child("centers");

    const int rows = centers_node.child("rows").text().as_int();
    const int cols = centers_node.child("cols").text().as_int();

    const auto left_top_node = centers_node.child("ltop");
    const double left_top_x = left_top_node.child("x").text().as_double();
    const double left_top_y = left_top_node.child("y").text().as_double();
    const cv::Point2d left_top{left_top_x, left_top_y};

    const auto right_top_node = centers_node.child("rtop");
    const double right_top_x = right_top_node.child("x").text().as_double();
    const double right_top_y = right_top_node.child("y").text().as_double();
    const cv::Point2d right_top{right_top_x, right_top_y};

    const auto left_bottom_node = centers_node.child("lbot");
    const double left_bottom_x = left_bottom_node.child("x").text().as_double();
    const double left_bottom_y = left_bottom_node.child("y").text().as_double();
    const cv::Point2d left_bottom{left_bottom_x, left_bottom_y};

    return {left_top, right_top, left_bottom, rows, cols, diameter, rotation};
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

} // namespace tlct::cfg::tspc
