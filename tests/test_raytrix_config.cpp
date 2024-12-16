#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::raytrix;

TEST_CASE("tlct::cfg::raytrix")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    const auto map = tlct::ConfigMap::fromPath("test/raytrix.cfg");
    auto layout = tn::Layout::fromCfgMap(map);

    SUBCASE("Layout")
    {
        constexpr double eps = 0.1;

        CHECK(layout.getImgWidth() == 6464);
        CHECK(layout.getImgHeight() == 4852);
        CHECK(layout.getImgSize() == cv::Size(6464, 4852));

        CHECK(layout.getDiameter() == doctest::Approx(37.154060363770).epsilon(eps));
        CHECK(layout.getRadius() == doctest::Approx(18.577030181885).epsilon(eps));
        CHECK(layout.isTranspose() == false);

        const auto center_0_0 = layout.getMICenter(0, 0);
        CHECK(center_0_0.x == doctest::Approx(48.4).epsilon(eps));
        CHECK(center_0_0.y == doctest::Approx(36.3).epsilon(eps));
        const auto center_1_0 = layout.getMICenter(1, 0);
        CHECK(center_1_0.x == doctest::Approx(29.8).epsilon(eps));
        CHECK(center_1_0.y == doctest::Approx(68.4).epsilon(eps));
        const auto center_0_1 = layout.getMICenter(0, 1);
        CHECK(center_0_1.x == doctest::Approx(85.5).epsilon(eps));
        CHECK(center_0_1.y == doctest::Approx(36.3).epsilon(eps));

        CHECK(layout.getMICenter({0, 0}) == center_0_0);
        CHECK(layout.getMICenter({1, 0}) == center_0_1);
        CHECK(layout.getMICenter({0, 1}) == center_1_0);

        CHECK(layout.getMIRows() == 150);
        CHECK(layout.getMIMinCols() == 173);
    }
}
