#include <filesystem>

#include <gtest/gtest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;
namespace tn = tlct::tspc;

class TestTSPCCfg : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const fs::path testdata_dir{TLCT_TESTDATA_DIR};
        const fs::path param_cfg_path = testdata_dir / "config/TSPC/param.cfg";
        const fs::path calib_cfg_path = testdata_dir / "config/TSPC/calib.xml";

        auto cfg_map = tlct::ConfigMap::fromPath(param_cfg_path.string());
        auto param_cfg = tn::ParamConfig::fromConfigMap(cfg_map);
        auto calib_cfg = tn::CalibConfig::fromXMLPath(calib_cfg_path.string());
        auto layout = tn::Layout::fromCfgAndImgsize(calib_cfg, param_cfg.getSpecificCfg().getImgSize());

        common_cfg_ = std::make_unique<decltype(cfg_map)>(std::move(cfg_map));
        param_cfg_ = std::make_unique<decltype(param_cfg)>(std::move(param_cfg));
        calib_cfg_ = std::make_unique<decltype(calib_cfg)>(calib_cfg);
        layout_ = std::make_unique<decltype(layout)>(layout);
    }

    static std::unique_ptr<tlct::cfg::ConfigMap> common_cfg_;
    static std::unique_ptr<tn::ParamConfig> param_cfg_;
    static std::unique_ptr<tn::CalibConfig> calib_cfg_;
    static std::unique_ptr<tn::Layout> layout_;
};

std::unique_ptr<tlct::cfg::ConfigMap> TestTSPCCfg::common_cfg_ = nullptr;
std::unique_ptr<tn::ParamConfig> TestTSPCCfg::param_cfg_ = nullptr;
std::unique_ptr<tn::CalibConfig> TestTSPCCfg::calib_cfg_ = nullptr;
std::unique_ptr<tn::Layout> TestTSPCCfg::layout_ = nullptr;

TEST_F(TestTSPCCfg, Param)
{
    const auto& param_cfg = *param_cfg_;

    EXPECT_EQ(param_cfg.getGenericCfg().getViews(), 5);
    EXPECT_EQ(param_cfg.getGenericCfg().getRange(), cv::Range(1, 2));
    EXPECT_EQ(param_cfg.getSpecificCfg().getImgSize(), cv::Size(4080, 3068));
    const auto fmt_src = param_cfg.getGenericCfg().fmtSrcPath(25);
    EXPECT_STREQ(fmt_src.string().c_str(), "./Cars/src/frame025.png");
    const auto fmt_dst = param_cfg.getGenericCfg().fmtDstPath(25);
    EXPECT_STREQ(fmt_dst.string().c_str(), "./Cars/dst/frame025");
}

TEST_F(TestTSPCCfg, Layout)
{
    const auto& layout = *layout_;

    EXPECT_EQ(layout.getImgWidth(), 3068);
    EXPECT_EQ(layout.getImgHeight(), 4080);
    EXPECT_EQ(layout.getImgSize(), cv::Size(3068, 4080));

    EXPECT_FLOAT_EQ(layout.getDiameter(), 70.);
    EXPECT_FLOAT_EQ(layout.getRadius(), 35.);
    EXPECT_FLOAT_EQ(layout.getRotation(), 1.57079632679);

    const auto center_0_0 = layout.getMICenter(0, 0);
    EXPECT_NEAR(center_0_0.x, 37.5, 0.5);
    EXPECT_NEAR(center_0_0.y, 38.25, 0.5);
    const auto center_1_0 = layout.getMICenter(1, 0);
    EXPECT_NEAR(center_1_0.x, 73., 0.5);
    EXPECT_NEAR(center_1_0.y, 99., 0.5);
    const auto center_0_1 = layout.getMICenter(0, 1);
    EXPECT_NEAR(center_0_1.x, 108., 0.5);
    EXPECT_NEAR(center_0_1.y, 38., 0.5);

    EXPECT_EQ(layout.getMICenter({0, 0}), center_0_0);
    EXPECT_EQ(layout.getMICenter({1, 0}), center_0_1);
    EXPECT_EQ(layout.getMICenter({0, 1}), center_1_0);

    EXPECT_EQ(layout.getMIRows(), 66);
    EXPECT_EQ(layout.getMIMinCols(), 43);
}
