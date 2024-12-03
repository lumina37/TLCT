#include <filesystem>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::tspc;

TEST_CASE("tlct::cfg::tspc")
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    fs::current_path(testdata_dir);

    auto cfg_map = tlct::ConfigMap::fromPath("test/param.cfg");
    auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
    const auto& calib_cfg = param_cfg.getCalibCfg();
    const auto& generic_cfg = param_cfg.getGenericCfg();
    const auto& spec_cfg = param_cfg.getSpecificCfg();
    auto layout = tn::Layout::fromCalibAndSpecConfig(calib_cfg, spec_cfg);

    SUBCASE("GenericCfg")
    {
        CHECK(generic_cfg.getViews() == 5);
        CHECK(generic_cfg.getRange() == cv::Range(1, 2));
        const auto& src_path = generic_cfg.getSrcPath();
        CHECK(src_path.string().c_str() == "./Cars/src.yuv");
        const auto& dst_path = generic_cfg.getDstPath();
        CHECK(dst_path.string().c_str() == "./Cars/dst");
    }

    SUBCASE("SpecificConfig")
    {
        constexpr double eps = 1e-3;

        CHECK(spec_cfg.getUpsample() == 2);
        CHECK(spec_cfg.getPsizeInflate() == doctest::Approx(2.0).epsilon(eps));
        CHECK(spec_cfg.getMaxPsize() == doctest::Approx(0.5).epsilon(eps));
        CHECK(spec_cfg.getPatternSize() == doctest::Approx(0.3).epsilon(eps));
        CHECK(spec_cfg.getPsizeShortcutThreshold() == 4);
    }

    SUBCASE("Layout")
    {
        constexpr double eps = 0.1;

        CHECK(layout.getImgWidth() == 3068);
        CHECK(layout.getImgHeight() == 4080);
        CHECK(layout.getImgSize() == cv::Size(3068, 4080));

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
        CHECK(layout.getMIMinCols() == 42);
    }
}
