#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "tlct.hpp"

#ifndef TLCT_TESTDATA_DIR
#    define TLCT_TESTDATA_DIR "."
#endif

namespace fs = std::filesystem;

TEST_CASE("Arrange with four corners", "tlct::cfg#CornersArrange") {
    const fs::path testdataDir{TLCT_TESTDATA_DIR};
    fs::current_path(testdataDir);

    const auto calibCfg = tlct::ConfigMap::createFromPath("test/清华单聚焦光场相机.cfg").value();
    const auto arrange = tlct::cfg::CornersArrange::createWithCalibCfg(calibCfg).value();

    constexpr float eps = 0.1f;

    REQUIRE(arrange.getImgWidth() == 3068);
    REQUIRE(arrange.getImgHeight() == 4080);
    REQUIRE(arrange.getImgSize() == cv::Size(3068, 4080));

    REQUIRE_THAT(arrange.getDiameter(), Catch::Matchers::WithinAbs(70., eps));
    REQUIRE_THAT(arrange.getRadius(), Catch::Matchers::WithinAbs(35., eps));
    REQUIRE(arrange.getDirection() == true);
    REQUIRE(arrange.isKepler() == true);
    REQUIRE(arrange.isMultiFocus() == false);

    const cv::Point2f& center_0_0 = arrange.getMICenter(0, 0);
    REQUIRE_THAT(center_0_0.x, Catch::Matchers::WithinAbs(37.5, eps));
    REQUIRE_THAT(center_0_0.y, Catch::Matchers::WithinAbs(38.25, eps));
    const cv::Point2f& center_1_0 = arrange.getMICenter(1, 0);
    REQUIRE_THAT(center_1_0.x, Catch::Matchers::WithinAbs(73.3, eps));
    REQUIRE_THAT(center_1_0.y, Catch::Matchers::WithinAbs(99.2, eps));
    const cv::Point2f& center_0_1 = arrange.getMICenter(0, 1);
    REQUIRE_THAT(center_0_1.x, Catch::Matchers::WithinAbs(108.0, eps));
    REQUIRE_THAT(center_0_1.y, Catch::Matchers::WithinAbs(38.2, eps));

    REQUIRE(arrange.getMICenter({0, 0}) == center_0_0);
    REQUIRE(arrange.getMICenter({1, 0}) == center_0_1);
    REQUIRE(arrange.getMICenter({0, 1}) == center_1_0);

    REQUIRE(arrange.getMIRows() == 66);
    REQUIRE(arrange.getMIMinCols() == 42);
}
