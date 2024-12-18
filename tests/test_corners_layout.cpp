#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

TEST_CASE("tlct::cfg#CornersLayout")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    const auto map = tlct::ConfigMap::fromPath("test/清华单聚焦光场相机.cfg");
    auto layout = tlct::cfg::CornersLayout::fromCfgMap(map);

    constexpr double eps = 0.1;

    CHECK(layout.getImgWidth() == 3068);
    CHECK(layout.getImgHeight() == 4080);
    CHECK(layout.getImgSize() == cv::Size(3068, 4080));

    CHECK(layout.getDiameter() == doctest::Approx(70.).epsilon(eps));
    CHECK(layout.getRadius() == doctest::Approx(35.).epsilon(eps));
    CHECK(layout.isTranspose() == true);

    const auto center_0_0 = layout.getMICenter(0, 0);
    CHECK(center_0_0.x == doctest::Approx(37.5).epsilon(eps));
    CHECK(center_0_0.y == doctest::Approx(38.25).epsilon(eps));
    const auto center_1_0 = layout.getMICenter(1, 0);
    CHECK(center_1_0.x == doctest::Approx(73.0).epsilon(eps));
    CHECK(center_1_0.y == doctest::Approx(99.0).epsilon(eps));
    const auto center_0_1 = layout.getMICenter(0, 1);
    CHECK(center_0_1.x == doctest::Approx(108.0).epsilon(eps));
    CHECK(center_0_1.y == doctest::Approx(38.0).epsilon(eps));

    CHECK(layout.getMICenter({0, 0}) == center_0_0);
    CHECK(layout.getMICenter({1, 0}) == center_0_1);
    CHECK(layout.getMICenter({0, 1}) == center_1_0);

    CHECK(layout.getMIRows() == 66);
    CHECK(layout.getMIMinCols() == 42);
}
