#include <filesystem>

#include <gtest/gtest.h>

#include "tlct/common/cmake.h"
#include "tlct/config/common.hpp"

namespace fs = std::filesystem;
using namespace tlct;

TEST(CommonParamConfig, TSPC)
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    const fs::path cfg_path = testdata_dir / "config/TSPC/param.cfg";
    const auto config = cfg::CommonParamConfig::fromPath(cfg_path.string());

    EXPECT_FALSE(config.isEmpty());
    EXPECT_EQ(config.getPipelineType(), cfg::PipelineType::TLCT);
}
