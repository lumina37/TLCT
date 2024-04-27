#include <vector>

#include <gtest/gtest.h>

#include "tlct/config/calibration/raytrix.hpp"

using namespace tlct;

TEST(RaytrixConfig, calibration)
{
    const auto config = cfg::raytrix::CalibConfig::fromPath("calibration.xml");

    EXPECT_FLOAT_EQ(config.getDiameter(), 23.202295303345);
    EXPECT_FLOAT_EQ(config.getRotation(), 1.570796370506);
    EXPECT_TRUE(config.isRotated());
    const auto offset = config.getRawOffset();
    EXPECT_FLOAT_EQ(offset.x, 13.040943844572);
    EXPECT_FLOAT_EQ(offset.y, -20.348051920159);
}
