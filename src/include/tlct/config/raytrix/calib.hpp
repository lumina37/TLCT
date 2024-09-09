#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <string>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"

namespace tlct::_cfg::raytrix {

constexpr int LEN_TYPE_NUM = 3;

class CalibConfig
{
public:
    // Typename alias
    using LenOffsets = std::array<int, LEN_TYPE_NUM>;

    // Constructor
    TLCT_API inline CalibConfig() noexcept : diameter_(), rotation_(), offset_(), lofs_(){};
    TLCT_API inline CalibConfig(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig& operator=(const CalibConfig& rhs) noexcept = default;
    TLCT_API inline CalibConfig(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig& operator=(CalibConfig&& rhs) noexcept = default;
    TLCT_API inline CalibConfig(double diameter, double rotation, cv::Point2d offset, LenOffsets lens) noexcept
        : diameter_(diameter), rotation_(rotation), offset_(offset), lofs_(lens){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLDoc(const pugi::xml_document& doc);
    [[nodiscard]] TLCT_API static inline CalibConfig fromXMLPath(std::string_view path);

    // Const methods
    [[nodiscard]] TLCT_API inline double getDiameter() const noexcept { return diameter_; };
    [[nodiscard]] TLCT_API inline double getRotation() const noexcept { return rotation_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getOffset() const noexcept { return offset_; };
    [[nodiscard]] TLCT_API inline LenOffsets getLenOffsets() const noexcept { return lofs_; };

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
    if (data_node.empty()) {
        std::cerr << "Missing xml node `RayCalibData` when initializing " << typeid(CalibConfig).name() << std::endl;
        return {};
    }

    const auto offset_node = data_node.child("offset");
    const double offset_x = offset_node.child("x").text().as_double();
    const double offset_y = offset_node.child("y").text().as_double();
    const cv::Point2d offset = {offset_x, offset_y};
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    LenOffsets lofs;
    for (const auto ltype_node : data_node.children("lens_type")) {
        const auto loffset_node = ltype_node.child("offset"); // Len Type Offset
        const int lid = ltype_node.attribute("id").as_int();
        const double loffset_x = loffset_node.child("x").text().as_double();
        const double loffset_y = loffset_node.child("y").text().as_double();
        const int loffset = (int)std::round(loffset_x + loffset_y);
        lofs[lid] = loffset;
    }

    return {diameter, rotation, offset, lofs};
}

CalibConfig CalibConfig::fromXMLPath(std::string_view path)
{
    pugi::xml_document doc;
    const auto ret = doc.load_file(path.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        std::cerr << "Failed to load `raytrix::CalibConfig` from `" << path << "`!" << std::endl;
        return {};
    }
    return CalibConfig::fromXMLDoc(doc);
}

} // namespace tlct::_cfg::raytrix
