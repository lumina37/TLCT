#include <filesystem>

#include <gtest/gtest.h>

#include "tlct/common/cmake.h"
#include "tlct/config/tspc.hpp"

using namespace tlct;
namespace fs = std::filesystem;

class TSPCConfig : public ::testing::Test
{
protected:
    static void SetUpTestCase()
    {
        const fs::path testdata_dir{TLCT_TESTDATA_DIR};
        const fs::path param_cfg_path = testdata_dir / "config/TSPC/param.cfg";
        const fs::path calib_cfg_path = testdata_dir / "config/TSPC/calib-coords.xml";

        auto common_cfg = cfg::CommonParamConfig::fromPath(param_cfg_path.string().c_str());
        auto param_cfg = cfg::tspc::ParamConfig::fromCommonCfg(common_cfg);
        auto calib_cfg = cfg::tspc::CalibConfig::fromXMLPath(calib_cfg_path.string().c_str());
        auto layout = cfg::tspc::Layout::fromCfgAndImgsize(calib_cfg, param_cfg.getImgSize());

        common_cfg_ = std::make_unique<decltype(common_cfg)>(std::move(common_cfg));
        param_cfg_ = std::make_unique<decltype(param_cfg)>(std::move(param_cfg));
        calib_cfg_ = std::make_unique<decltype(calib_cfg)>(std::move(calib_cfg));
        layout_ = std::make_unique<decltype(layout)>(std::move(layout));
    }

    static std::unique_ptr<cfg::CommonParamConfig> common_cfg_;
    static std::unique_ptr<cfg::ParamConfig> param_cfg_;
    static std::unique_ptr<cfg::CalibConfig> calib_cfg_;
    static std::unique_ptr<cfg::Layout> layout_;
};

std::unique_ptr<cfg::CommonParamConfig> TSPCConfig::common_cfg_ = nullptr;
std::unique_ptr<cfg::ParamConfig> TSPCConfig::param_cfg_ = nullptr;
std::unique_ptr<cfg::CalibConfig> TSPCConfig::calib_cfg_ = nullptr;
std::unique_ptr<cfg::Layout> TSPCConfig::layout_ = nullptr;

TEST_F(TSPCConfig, Param)
{
    const auto& param_cfg = *param_cfg_;

    EXPECT_EQ(param_cfg.getViews(), 5);
    EXPECT_EQ(param_cfg.getImgSize(), cv::Size(4080, 3068));
    EXPECT_EQ(param_cfg.getRange(), cv::Range(0, 1));
    const auto fmt_src = cfg::fmtSrcPath(param_cfg, 25);
    EXPECT_STREQ(fmt_src.string().c_str(), "./Cars/src/frame025.png");
    const auto fmt_dst = cfg::fmtDstPath(param_cfg, 25);
    EXPECT_STREQ(fmt_dst.string().c_str(), "./Cars/dst/frame025");
}

TEST_F(TSPCConfig, Layout)
{
    const auto& layout = *layout_;

    EXPECT_FLOAT_EQ(layout.getDiameter(), 70.0);
    EXPECT_FLOAT_EQ(layout.getRotation(), 1.57079632679);
}
