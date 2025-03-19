#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "tlct.hpp"

#ifndef TLCT_TESTDATA_DIR
#    define TLCT_TESTDATA_DIR "."
#endif

namespace fs = std::filesystem;

TEST_CASE("Arrange with central MI and offset", "tlct::cfg#OffsetArrange") {
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    const auto& cfgMap = tlct::ConfigMap::fromPath("test/raytrix.cfg");
    const auto& arrange = tlct::cfg::OffsetArrange::fromCfgMap(cfgMap);

    constexpr float eps = 0.1f;

    REQUIRE(arrange.getImgWidth() == 6464);
    REQUIRE(arrange.getImgHeight() == 4852);
    REQUIRE(arrange.getImgSize() == cv::Size(6464, 4852));

    REQUIRE_THAT(arrange.getDiameter(), Catch::Matchers::WithinAbs(37.154060363770, eps));
    REQUIRE_THAT(arrange.getRadius(), Catch::Matchers::WithinAbs(18.577030181885, eps));
    REQUIRE(arrange.getDirection() == false);

    const cv::Point2f& center_0_0 = arrange.getMICenter(0, 0);
    REQUIRE_THAT(center_0_0.x, Catch::Matchers::WithinAbs(48.4, eps));
    REQUIRE_THAT(center_0_0.y, Catch::Matchers::WithinAbs(36.3, eps));
    const cv::Point2f& center_1_0 = arrange.getMICenter(1, 0);
    REQUIRE_THAT(center_1_0.x, Catch::Matchers::WithinAbs(29.8, eps));
    REQUIRE_THAT(center_1_0.y, Catch::Matchers::WithinAbs(68.4, eps));
    const cv::Point2f& center_0_1 = arrange.getMICenter(0, 1);
    REQUIRE_THAT(center_0_1.x, Catch::Matchers::WithinAbs(85.5, eps));
    REQUIRE_THAT(center_0_1.y, Catch::Matchers::WithinAbs(36.3, eps));

    REQUIRE(arrange.getMICenter({0, 0}) == center_0_0);
    REQUIRE(arrange.getMICenter({1, 0}) == center_0_1);
    REQUIRE(arrange.getMICenter({0, 1}) == center_1_0);

    REQUIRE(arrange.getMIRows() == 150);
    REQUIRE(arrange.getMIMinCols() == 173);
}
