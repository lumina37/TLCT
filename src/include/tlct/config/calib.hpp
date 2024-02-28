#include <string>
#include <vector>

#include <pugixml.hpp>

#include <opencv2/core.hpp>

namespace tlct::
inline config {

class CalibConfig {
public:
    CalibConfig() : offset_(0, 0), diameter_(0.0), rotation_(0.0), lens_() {};

    CalibConfig(cv::Point2d offset,
                double diameter,
                double rotation,
                std::vector<cv::Point2d> &&lens) :
            offset_(offset),
            diameter_(diameter),
            rotation_(rotation),
            lens_(lens) {};

    static CalibConfig fromPath(const std::string &xml_fpath);

    double getDiameter() const noexcept;

    cv::Point2d getOffset() const noexcept;

private:
    cv::Point2d offset_;
    double diameter_;
    double rotation_;
    std::vector<cv::Point2d> lens_;
};


inline CalibConfig CalibConfig::fromPath(const std::string &xml_fpath) {
    pugi::xml_document doc;
    const auto ret = doc.load_file(xml_fpath.c_str(), pugi::parse_minimal, pugi::encoding_utf8);
    if (!ret) {
        return {};
    }

    const auto data_node = doc.child("RayCalibData");
    const auto offset_node = data_node.child("offset");
    const double offset_x = offset_node.child("x").text().as_double();
    const double offset_y = offset_node.child("y").text().as_double();
    const cv::Point2d offset = {offset_x, offset_y};
    const auto diameter = data_node.child("diameter").text().as_double();
    const auto rotation = data_node.child("rotation").text().as_double();

    std::vector<cv::Point2d> lens;
    constexpr size_t type_num = 3;
    lens.reserve(type_num);
    for (const auto ltype_node: data_node.children("lens_type")) {
        const auto lofs = ltype_node.child("offset");  // Len Type Offset
        const double lofs_x = lofs.child("x").text().as_double();
        const double lofs_y = lofs.child("y").text().as_double();
        lens.emplace_back(lofs_x, lofs_y);
    }

    return {offset, diameter, rotation, std::move(lens)};
}

inline double CalibConfig::getDiameter() const noexcept {
    return diameter_;
}

inline cv::Point2d CalibConfig::getOffset() const noexcept {
    return offset_;
}

}
