#pragma once

#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts/calib.hpp"

namespace tlct::cfg::raytrix {

constexpr int LEN_TYPE_NUM = 3;
using LenOffsets = cv::Vec<cv::Point2d, LEN_TYPE_NUM>;

class CalibConfig
{
public:
    friend class Layout;

    // Constructor
    TLCT_API inline CalibConfig() noexcept : diameter_(), rotation_(), offset_(){};
    TLCT_API inline CalibConfig& operator=(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig& operator=(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig(double diameter, double rotation, cv::Point2d offset, const LenOffsets& lens) noexcept
        : diameter_(diameter), rotation_(rotation), offset_(offset), lofs_(lens){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLPath(const std::string_view& path);

private:
    double diameter_;
    double rotation_;
    cv::Point2d offset_; // be careful: (x,-y) is the corresponding coord repr in OpenCV
    LenOffsets lofs_;
};

static_assert(concepts::CCalibConfig<CalibConfig>);

CalibConfig CalibConfig::fromXMLDoc(const pugi::xml_document& doc)
{
    const auto data_node = doc.child("RayCalibData");
    const auto offset_node = data_node.child("offset");
    const double offset_x = offset_node.child("x").text().as_double();
    const double offset_y = offset_node.child("y").text().as_double();
    const cv::Point2d offset = {offset_x, offset_y};
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    LenOffsets lofs;
    for (const auto ltype_node : data_node.children("lens_type")) {
        const auto loffset = ltype_node.child("offset"); // Len Type Offset
        const int lid = ltype_node.attribute("id").as_int();
        const double loffset_x = loffset.child("x").text().as_double();
        const double loffset_y = loffset.child("y").text().as_double();
        lofs[lid] = {loffset_x, loffset_y};
    }

    return {diameter, rotation, offset, lofs};
}

CalibConfig CalibConfig::fromXMLPath(const std::string_view& path)
{
    pugi::xml_document doc;
    const auto ret = doc.load_file(path.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        std::cerr << "Failed to load `" << typeid(CalibConfig).name() << "` from `" << path << "`!" << std::endl;
        return {};
    }
    return CalibConfig::fromXMLDoc(doc);
}

} // namespace tlct::cfg::raytrix
