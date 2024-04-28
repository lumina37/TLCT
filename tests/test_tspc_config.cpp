#include <gtest/gtest.h>

#include "tlct/config/calibration/tspc.hpp"

using namespace tlct;

TEST(TSPCConfig, calibration)
{
    cfg::tspc::CalibConfig config{};
    config._setCenters("centers_cars.xml");

    const auto first_center = config.getCenter(0, 0);
    const cv::Point2i expect_first_center{44, 42};
    EXPECT_EQ(first_center, expect_first_center);

    const auto size=config.getCentersSize();
    const cv::Size expect_size {66,43};
    EXPECT_EQ(size, expect_size);
}
