#pragma once

#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"

namespace tlct::cfg::raytrix {

class CalibConfig
{
public:
    TLCT_API CalibConfig() : offset_(0, 0), diameter_(), rotation_() {};
    TLCT_API CalibConfig(cv::Point2d offset, double diameter, double rotation)
        : offset_(offset), diameter_(diameter), rotation_(rotation) {};

    [[nodiscard]] TLCT_API static CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static CalibConfig fromXMLPath(const char* path);

    [[nodiscard]] TLCT_API double getDiameter() const noexcept;
    [[nodiscard]] TLCT_API double getRotation() const noexcept;
    [[nodiscard]] TLCT_API bool isRotated() const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getRawOffset() const noexcept;
    [[nodiscard]] TLCT_API cv::Point2d getOffset() const noexcept;

private:
    cv::Point2d offset_;
    double diameter_;
    double rotation_;
};

inline CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{
    const auto data_node = doc.child("RayCalibData");
    const auto offset_node = data_node.child("offset");
    const double offset_x = offset_node.child("x").text().as_double();
    const double offset_y = offset_node.child("y").text().as_double();
    const cv::Point2d offset = {offset_x, offset_y};
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    std::vector<cv::Point2d> lens;
    constexpr size_t type_num = 3;
    lens.reserve(type_num);
    for (const auto ltype_node : data_node.children("lens_type")) {
        const auto lofs = ltype_node.child("offset"); // Len Type Offset
        const double lofs_x = lofs.child("x").text().as_double();
        const double lofs_y = lofs.child("y").text().as_double();
        lens.emplace_back(lofs_x, lofs_y);
    }

    return {offset, diameter, rotation};
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

inline double CalibConfig::getDiameter() const noexcept { return diameter_; }

inline double CalibConfig::getRotation() const noexcept { return rotation_; }

inline bool CalibConfig::isRotated() const noexcept { return rotation_ != 0.0; }

inline cv::Point2d CalibConfig::getRawOffset() const noexcept { return offset_; }

inline cv::Point2d CalibConfig::getOffset() const noexcept { return {offset_.x, -offset_.y}; }

} // namespace tlct::cfg::raytrix
