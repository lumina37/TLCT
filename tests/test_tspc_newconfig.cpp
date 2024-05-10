#include <gtest/gtest.h>

#include "tlct/newconfig/tspc.hpp"

using namespace tlct;

TEST(NewTSPCConfig, calibration)
{
    cfg::tspc::CalibConfig config{}; // TODO: Add tests for diameter, rotation etc.
    config._setCenters("centers_cars.xml");

    const auto center_r0c0 = config.getMICenter(0, 0);
//    const cv::Point expect_center_r0c0{44, 42};
//    EXPECT_EQ(center_r0c0, expect_center_r0c0);
//
//    const auto center_r0c1 = config.getMICenter(0, 1);
//    const cv::Point expect_center_r0c1{105, 77};
//    EXPECT_EQ(center_r0c1, expect_center_r0c1);
//
//    const auto center_r1c0 = config.getMICenter(1, 0);
//    const cv::Point expect_center_r1c0{44, 112};
//    EXPECT_EQ(center_r1c0, expect_center_r1c0);

    const auto size = config.getMINums();
    const cv::Size expect_size{67, 43};
    EXPECT_EQ(size, expect_size);
}
