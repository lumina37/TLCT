#include <filesystem>

#include <gtest/gtest.h>

#include "tlct/common/cmake.h"
#include "tlct/config/tspc.hpp"

using namespace tlct::cfg::tspc::v1;
namespace fs = std::filesystem;

class TSPCConfig : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const fs::path testdata_dir{TLCT_TESTDATA_DIR};
        const fs::path param_cfg_path = testdata_dir / "config/TSPC/param.cfg";
        const fs::path calib_cfg_path = testdata_dir / "config/TSPC/calib-coords.xml";

        auto common_cfg = tlct::cfg::CommonParamConfig::fromPath(param_cfg_path.string().c_str());
        auto param_cfg = ParamConfig::fromCommonCfg(common_cfg);
        auto calib_cfg = CalibConfig::fromXMLPath(calib_cfg_path.string().c_str());
        auto layout = Layout::fromCfgAndImgsize(calib_cfg, param_cfg.getImgSize());

        common_cfg_ = std::make_unique<decltype(common_cfg)>(std::move(common_cfg));
        param_cfg_ = std::make_unique<decltype(param_cfg)>(std::move(param_cfg));
        calib_cfg_ = std::make_unique<decltype(calib_cfg)>(std::move(calib_cfg));
        layout_ = std::make_unique<decltype(layout)>(std::move(layout));
    }

    static std::unique_ptr<tlct::cfg::CommonParamConfig> common_cfg_;
    static std::unique_ptr<ParamConfig> param_cfg_;
    static std::unique_ptr<CalibConfig> calib_cfg_;
    static std::unique_ptr<Layout> layout_;
};

std::unique_ptr<tlct::cfg::CommonParamConfig> TSPCConfig::common_cfg_ = nullptr;
std::unique_ptr<ParamConfig> TSPCConfig::param_cfg_ = nullptr;
std::unique_ptr<CalibConfig> TSPCConfig::calib_cfg_ = nullptr;
std::unique_ptr<Layout> TSPCConfig::layout_ = nullptr;

TEST_F(TSPCConfig, Param)
{
    const auto& param_cfg = *param_cfg_;

    EXPECT_EQ(param_cfg.getViews(), 5);
    EXPECT_EQ(param_cfg.getImgSize(), cv::Size(4080, 3068));
    EXPECT_EQ(param_cfg.getRange(), cv::Range(0, 1));
    const auto fmt_src = fmtSrcPath(param_cfg, 25);
    EXPECT_STREQ(fmt_src.string().c_str(), "./Cars/src/frame025.png");
    const auto fmt_dst = fmtDstPath(param_cfg, 25);
    EXPECT_STREQ(fmt_dst.string().c_str(), "./Cars/dst/frame025");
}

TEST_F(TSPCConfig, Layout)
{
    const auto& layout = *layout_;

    EXPECT_EQ(layout.getImgWidth(), 3068);
    EXPECT_EQ(layout.getImgHeight(), 4080);
    EXPECT_EQ(layout.getImgSize(), cv::Size(3068, 4080));

    EXPECT_FLOAT_EQ(layout.getDiameter(), 70.);
    EXPECT_FLOAT_EQ(layout.getRadius(), 35.);
    EXPECT_FLOAT_EQ(layout.getRotation(), 1.57079632679);

    const auto center0_0_0 = layout.getMICenter(0, 0);
    EXPECT_NEAR(center0_0_0.x, 38., 0.5);
    EXPECT_NEAR(center0_0_0.y, 37., 0.5);
    const auto center0_1_0 = layout.getMICenter(1, 0);
    EXPECT_NEAR(center0_1_0.x, 73., 0.5);
    EXPECT_NEAR(center0_1_0.y, 98., 0.5);
    const auto center0_0_1 = layout.getMICenter(0, 1);
    EXPECT_NEAR(center0_0_1.x, 108., 0.5);
    EXPECT_NEAR(center0_0_1.y, 37., 0.5);

    EXPECT_EQ(layout.getMICenter({0, 0}), center0_0_0);
    EXPECT_EQ(layout.getMICenter({1, 0}), center0_0_1);
    EXPECT_EQ(layout.getMICenter({0, 1}), center0_1_0);
}
