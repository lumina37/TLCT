#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

#ifndef TLCT_TESTDATA_DIR
#    define TLCT_TESTDATA_DIR "."
#endif

namespace fs = std::filesystem;

TEST_CASE("tlct::cfg#OffsetArrange") {
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    const auto& cfgMap = tlct::ConfigMap::fromPath("test/raytrix.cfg");
    const auto& arrange = tlct::cfg::OffsetArrange::fromCfgMap(cfgMap);

    constexpr float eps = 0.1;

    CHECK(arrange.getImgWidth() == 6464);
    CHECK(arrange.getImgHeight() == 4852);
    CHECK(arrange.getImgSize() == cv::Size(6464, 4852));

    CHECK(arrange.getDiameter() == doctest::Approx(37.154060363770).epsilon(eps));
    CHECK(arrange.getRadius() == doctest::Approx(18.577030181885).epsilon(eps));
    CHECK(arrange.getDirection() == false);

    const cv::Point2f& center_0_0 = arrange.getMICenter(0, 0);
    CHECK(center_0_0.x == doctest::Approx(48.4).epsilon(eps));
    CHECK(center_0_0.y == doctest::Approx(36.3).epsilon(eps));
    const cv::Point2f& center_1_0 = arrange.getMICenter(1, 0);
    CHECK(center_1_0.x == doctest::Approx(29.8).epsilon(eps));
    CHECK(center_1_0.y == doctest::Approx(68.4).epsilon(eps));
    const cv::Point2f& center_0_1 = arrange.getMICenter(0, 1);
    CHECK(center_0_1.x == doctest::Approx(85.5).epsilon(eps));
    CHECK(center_0_1.y == doctest::Approx(36.3).epsilon(eps));

    CHECK(arrange.getMICenter({0, 0}) == center_0_0);
    CHECK(arrange.getMICenter({1, 0}) == center_0_1);
    CHECK(arrange.getMICenter({0, 1}) == center_1_0);

    CHECK(arrange.getMIRows() == 150);
    CHECK(arrange.getMIMinCols() == 173);
}
