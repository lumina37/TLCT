#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

TEST_CASE("tlct::cfg#OffsetLayout")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    const auto& cfg_map = tlct::ConfigMap::fromPath("test/raytrix.cfg");
    const auto& layout = tlct::cfg::OffsetLayout::fromCfgMap(cfg_map);

    constexpr double eps = 0.1;

    CHECK(layout.getImgWidth() == 6464);
    CHECK(layout.getImgHeight() == 4852);
    CHECK(layout.getImgSize() == cv::Size(6464, 4852));

    CHECK(layout.getDiameter() == doctest::Approx(37.154060363770).epsilon(eps));
    CHECK(layout.getRadius() == doctest::Approx(18.577030181885).epsilon(eps));
    CHECK(layout.getDirection() == false);

    const cv::Point2d& center_0_0 = layout.getMICenter(0, 0);
    CHECK(center_0_0.x == doctest::Approx(48.4).epsilon(eps));
    CHECK(center_0_0.y == doctest::Approx(36.3).epsilon(eps));
    const cv::Point2d& center_1_0 = layout.getMICenter(1, 0);
    CHECK(center_1_0.x == doctest::Approx(29.8).epsilon(eps));
    CHECK(center_1_0.y == doctest::Approx(68.4).epsilon(eps));
    const cv::Point2d& center_0_1 = layout.getMICenter(0, 1);
    CHECK(center_0_1.x == doctest::Approx(85.5).epsilon(eps));
    CHECK(center_0_1.y == doctest::Approx(36.3).epsilon(eps));

    CHECK(layout.getMICenter({0, 0}) == center_0_0);
    CHECK(layout.getMICenter({1, 0}) == center_0_1);
    CHECK(layout.getMICenter({0, 1}) == center_1_0);

    CHECK(layout.getMIRows() == 150);
    CHECK(layout.getMIMinCols() == 173);
}
