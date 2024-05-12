#include <vector>

#include <gtest/gtest.h>

#include "tlct/config/common.hpp"

using namespace tlct;

TEST(RaytrixConfig, calibration)
{
    const auto config = cfg::CommonParamConfig::fromPath("common.cfg");

    EXPECT_FALSE(config.isEmpty());
    EXPECT_TRUE(config.isTSPC());
}
