#include <filesystem>

#include <gtest/gtest.h>

#include "tlct/common/cmake.h"
#include "tlct/config/common.hpp"

using namespace tlct;
namespace fs = std::filesystem;

TEST(CommonParamConfig, TSPC)
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    const fs::path cfg_path = testdata_dir / "config/TSPC/param.cfg";
    const auto config = cfg::CommonParamConfig::fromPath(cfg_path.string().c_str());

    EXPECT_FALSE(config.isEmpty());
    EXPECT_TRUE(config.isTSPC());
}
