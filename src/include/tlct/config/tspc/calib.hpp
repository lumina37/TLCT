#pragma once

#include <exception>
#include <sstream>
#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"

namespace tlct::_cfg::tspc {

class CalibConfig
{
public:
    // Constructor
    TLCT_API inline CalibConfig() noexcept
        : diameter_(), rotation_(), left_top_(), right_top_(), left_bottom_(), right_bottom_(){};
    TLCT_API inline CalibConfig(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig& operator=(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig& operator=(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig(double diameter, double rotation, cv::Point2d left_top, cv::Point2d right_top,
                                cv::Point2d left_bottom, cv::Point2d right_bottom) noexcept
        : diameter_(diameter), rotation_(rotation), left_top_(left_top), right_top_(right_top),
          left_bottom_(left_bottom), right_bottom_(right_bottom){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLPath(std::string_view path);

    // Const methods
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRotation() const noexcept { return rotation_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getLeftTop() const noexcept { return left_top_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getRightTop() const noexcept { return right_top_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getLeftBottom() const noexcept { return left_bottom_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getRightBottom() const noexcept { return right_bottom_; };

private:
    cv::Point2d left_top_;
    cv::Point2d right_top_;
    cv::Point2d left_bottom_;
    cv::Point2d right_bottom_;
    double diameter_;
    double rotation_;
};



CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{
    const auto data_node = doc.child("TSPCCalibData");
    if (data_node.empty()) [[unlikely]] {
        throw std::runtime_error{"Missing xml node `TSPCCalibData` when initializing raytrix::CalibConfig"};
    }

    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    const auto centers_node = data_node.child("centers");

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

    const auto right_bottom_node = centers_node.child("rbot");
    const double right_bottom_x = right_bottom_node.child("x").text().as_double();
    const double right_bottom_y = right_bottom_node.child("y").text().as_double();
    const cv::Point2d right_bottom{right_bottom_x, right_bottom_y};

    return {diameter, rotation, left_top, right_top, left_bottom, right_bottom};
}

CalibConfig CalibConfig::fromXMLPath(std::string_view path)
{
    pugi::xml_document doc;
    const auto ret = doc.load_file(path.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) [[unlikely]] {
        std::stringstream err;
        err << "Failed to load `tspc::CalibConfig` from `" << path << "`!" << std::endl;
        throw std::runtime_error{err.str()};
    }
    return CalibConfig::fromXMLDoc(doc);
}

} // namespace tlct::_cfg::tspc
