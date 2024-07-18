#include <filesystem>

#include <gtest/gtest.h>

#include "tlct.hpp"

namespace fs = std::filesystem;

TEST(CommonParamConfig, TSPC)
{
    const fs::path testdata_dir{TLCT_TESTDATA_DIR};
    const fs::path cfg_path = testdata_dir / "config/TSPC/param.cfg";
    const auto config = tlct::ConfigMap::fromPath(cfg_path.string());

    EXPECT_FALSE(config.isEmpty());
    EXPECT_EQ(config.getPipelineType(), tlct::PipelineType::TLCT);
}
