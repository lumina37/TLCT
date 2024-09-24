#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::tspc;

TEST_CASE("cfg::tspc")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    auto cfg_map = tlct::ConfigMap::fromPath("config/TSPC/param.cfg");
    auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
    const auto& calib_cfg = param_cfg.getCalibCfg();
    auto layout = tn::Layout::fromParamConfig(param_cfg);

    SUBCASE("tspc::ParamConfig")
    {
        CHECK(param_cfg.getGenericCfg().getViews() == 5);
        CHECK(param_cfg.getGenericCfg().getRange() == cv::Range(1, 2));
        CHECK(param_cfg.getSpecificCfg().getImgSize() == cv::Size(4080, 3068));
        const auto fmt_src = param_cfg.getGenericCfg().fmtSrcPath(25);
        CHECK(fmt_src.string().c_str() == "./Cars/src/frame025.png");
        const auto fmt_dst = param_cfg.getGenericCfg().fmtDstPath(25);
        CHECK(fmt_dst.string().c_str() == "./Cars/dst/frame025");
    }

    SUBCASE("tspc::Layout")
    {
        CHECK(layout.getImgWidth() == 3068);
        CHECK(layout.getImgHeight() == 4080);
        CHECK(layout.getImgSize() == cv::Size(3068, 4080));

        constexpr double eps = 0.1;
        CHECK(layout.getDiameter() == doctest::Approx(70.).epsilon(eps));
        CHECK(layout.getRadius() == doctest::Approx(35.).epsilon(eps));
        CHECK(layout.getRotation() == doctest::Approx(1.57079632679).epsilon(eps));

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
        CHECK(layout.getMIMinCols() == 43);
    }
}
