#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "tlct/config/calib.hpp"

TEST(Config, calib)
{
    const auto config = tlct::CalibConfig::fromPath("calibration.xml");

    const auto offset = config.getOffset();
    EXPECT_FLOAT_EQ(offset.x, 13.040943844572);
    EXPECT_FLOAT_EQ(offset.y, -20.348051920159);
    EXPECT_FLOAT_EQ(config.getDiameter(), 23.202295303345);
}
