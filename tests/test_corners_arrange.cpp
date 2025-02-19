#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

#ifndef TLCT_TESTDATA_DIR
#    define TLCT_TESTDATA_DIR "."
#endif

namespace fs = std::filesystem;

TEST_CASE("tlct::cfg#CornersArrange") {
    const fs::path testdataDir{TLCT_TESTDATA_DIR};
    fs::current_path(testdataDir);

    const auto& cfgMap = tlct::ConfigMap::fromPath("test/清华单聚焦光场相机.cfg");
    const auto& arrange = tlct::cfg::CornersArrange::fromCfgMap(cfgMap);

    constexpr float eps = 0.1;

    CHECK(arrange.getImgWidth() == 3068);
    CHECK(arrange.getImgHeight() == 4080);
    CHECK(arrange.getImgSize() == cv::Size(3068, 4080));

    CHECK(arrange.getDiameter() == doctest::Approx(70.).epsilon(eps));
    CHECK(arrange.getRadius() == doctest::Approx(35.).epsilon(eps));
    CHECK(arrange.getDirection() == true);

    const cv::Point2f& center_0_0 = arrange.getMICenter(0, 0);
    CHECK(center_0_0.x == doctest::Approx(37.5).epsilon(eps));
    CHECK(center_0_0.y == doctest::Approx(38.25).epsilon(eps));
    const cv::Point2f& center_1_0 = arrange.getMICenter(1, 0);
    CHECK(center_1_0.x == doctest::Approx(73.0).epsilon(eps));
    CHECK(center_1_0.y == doctest::Approx(99.0).epsilon(eps));
    const cv::Point2f& center_0_1 = arrange.getMICenter(0, 1);
    CHECK(center_0_1.x == doctest::Approx(108.0).epsilon(eps));
    CHECK(center_0_1.y == doctest::Approx(38.0).epsilon(eps));

    CHECK(arrange.getMICenter({0, 0}) == center_0_0);
    CHECK(arrange.getMICenter({1, 0}) == center_0_1);
    CHECK(arrange.getMICenter({0, 1}) == center_1_0);

    CHECK(arrange.getMIRows() == 66);
    CHECK(arrange.getMIMinCols() == 42);
}
