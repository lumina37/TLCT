#pragma once

#include <ranges>
#include <string>

#include <pugixml.hpp>

#include <opencv2/core.hpp>

namespace tlct::cfg::tspc {

namespace rgs = std::ranges;

class CalibConfig
{
public:
    CalibConfig() : diameter_(0.0), rotation_(0.0), centers_(){};
    CalibConfig(double diameter, double rotation, cv::Mat&& centers)
        : diameter_(diameter), rotation_(rotation), centers_(centers){};

    static CalibConfig fromPath(const std::string_view xml_fpath);

    int _setCenters(std::string_view xml_fpath);

    [[nodiscard]] double getDiameter() const noexcept;
    [[nodiscard]] double getRotation() const noexcept;
    [[nodiscard]] bool isRotated() const noexcept;
    [[nodiscard]] cv::Point getCenter(int row, int col) const noexcept;
    [[nodiscard]] cv::Size getCentersSize() const noexcept;

private:
    double diameter_;
    double rotation_;
    cv::Mat centers_;
};

inline CalibConfig CalibConfig::fromPath(const std::string_view xml_fpath)
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

inline int CalibConfig::_setCenters(const std::string_view xml_fpath)
{
    // TODO: This incorporating method of the center map is ugly, try a better approach.

    pugi::xml_document doc;
    const auto ret = doc.load_file(xml_fpath.data(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return -1;
    }

    const auto data_node = doc.child("TSPCCalibData");
    const auto centermap_node = data_node.child("centers");
    int rows = centermap_node.child("rows").text().as_int();
    int cols = centermap_node.child("cols").text().as_int();

    const std::string coord_str = centermap_node.child("coords").text().as_string();
    auto subrg_view = coord_str | rgs::views::split(' ');
    auto subrg_iter = subrg_view.begin();

    const auto subrg2int = [](const auto& subrg) {
        const std::string s{subrg.begin(), subrg.end()};
        const int v = std::stoi(s);
        return v;
    };

    this->centers_.create(rows, cols, CV_32SC2);
    for (const int row : rgs::views::iota(0, rows)) {
        auto prow = this->centers_.ptr<cv::Point>(row);

        for (const int col : rgs::views::iota(0, cols)) {
            const int x = subrg2int(*subrg_iter);
            subrg_iter++;
            const int y = subrg2int(*subrg_iter);
            subrg_iter++;
            prow[col] = {x, y};
        }
    }

    return 0;
}

inline double CalibConfig::getDiameter() const noexcept { return diameter_; }

inline double CalibConfig::getRotation() const noexcept { return rotation_; }

inline bool CalibConfig::isRotated() const noexcept { return rotation_ != 0.0; }

inline cv::Point CalibConfig::getCenter(const int row, const int col) const noexcept
{
    return this->centers_.at<cv::Point>(row, col);
}

inline cv::Size CalibConfig::getCentersSize() const noexcept { return centers_.size(); }

} // namespace tlct::cfg::tspc
