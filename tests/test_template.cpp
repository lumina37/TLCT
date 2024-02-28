#include <string>
#include <vector>
#include <iostream>

#include <gtest/gtest.h>

#include "tlct/utils/xmlreader.hpp"

TEST(Template, exampleA) {
    std::vector<uint8_t> vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    cv::Mat src(3, 2, CV_8UC2, vec.data());

    EXPECT_EQ(src.rows, 3);
    EXPECT_EQ(src.cols, 2);
    EXPECT_EQ(src.channels(), 2);
    auto src_last_elem = src.at<cv::Vec2b>(2, 1)[1];
    EXPECT_EQ(src_last_elem, 12);

    auto filename = std::string("data.hdf5");
    tlct::toHdf5(filename, src);
    cv::Mat dst;
    tlct::fromHdf5(filename, dst);

    EXPECT_EQ(dst.rows, 3);
    EXPECT_EQ(dst.cols, 2);
    EXPECT_EQ(dst.channels(), 2);
    auto dst_last_elem = dst.at<cv::Vec2b>(2, 1)[1];
    EXPECT_EQ(dst_last_elem, 12);
}

TEST(Template, exampleB) {
    auto filename = std::string("bridge.hdf5");
    cv::Mat dst;
    tlct::fromHdf5(filename, dst);

    std::cout << dst;
}
