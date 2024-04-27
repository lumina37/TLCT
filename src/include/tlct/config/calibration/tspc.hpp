#pragma once

#include <string>

#include <pugixml.hpp>

#include <opencv2/core.hpp>

namespace tlct::cfg::tspc {

class CalibConfig
{
public:
    CalibConfig() : diameter_(0.0), rotation_(0.0), centers_(){};
    CalibConfig(double diameter, double rotation, cv::Mat&& centers)
        : diameter_(diameter), rotation_(rotation), centers_(centers){};

    static CalibConfig fromPath(std::string_view xml_fpath);

    int _setCenters(std::string_view xml_fpath);

    [[nodiscard]] double getDiameter() const noexcept;
    [[nodiscard]] double getRotation() const noexcept;
    [[nodiscard]] bool isRotated() const noexcept;
    [[nodiscard]] cv::Point2i getCenter(int row, int col) const noexcept;
    [[nodiscard]] cv::Size getCentersSize() const noexcept;

private:
    double diameter_;
    double rotation_;
    cv::Mat centers_;
};

inline CalibConfig CalibConfig::fromPath(std::string_view xml_fpath)
{
    pugi::xml_document doc;
    // TODO: Error if the string view `xml_fpath` is not ending with `\0`, try a safer approach.
    const auto ret = doc.load_file(xml_fpath.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return {};
    }

    const auto data_node = doc.child("RayCalibData");
    const double diameter = data_node.child("diameter").text().as_double();
    const double rotation = data_node.child("rotation").text().as_double();

    return {diameter, rotation, {}};
}

inline int CalibConfig::_setCenters(std::string_view xml_fpath)
{
    // TODO: This incorporating method of the center map is ugly, try a better approach.

    pugi::xml_document doc;
    const auto ret = doc.load_file(xml_fpath.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return -1;
    }

    const auto data_node = doc.child("RayCalibData");
    const auto centermap_node = data_node.child("Centermap");
    int height = centermap_node.child("height").text().as_int();
    int width = centermap_node.child("width").text().as_int();

    this->centers_.create(height, width, CV_32SC2);

    int index = 0;
    for (const auto center_node : centermap_node.children("Point")) {
        int x = center_node.child("x").text().as_int();
        int y = center_node.child("y").text().as_int();
        int row = index / width;
        int col = index % width;
        this->centers_.at<cv::Point2i>(row, col) = cv::Point2i(x, y);
        index++;
    }

    return 0;
}

inline double CalibConfig::getDiameter() const noexcept { return diameter_; }

inline double CalibConfig::getRotation() const noexcept { return rotation_; }

inline bool CalibConfig::isRotated() const noexcept { return rotation_ != 0.0; }

inline cv::Point2i CalibConfig::getCenter(int row, int col) const noexcept
{
    return this->centers_.at<cv::Point2i>(row, col);
}

inline cv::Size CalibConfig::getCentersSize() const noexcept { return centers_.size(); }

} // namespace tlct::cfg::tspc
