#include <ranges>

#include <gtest/gtest.h>
#include <opencv2/imgcodecs.hpp>

namespace rgs = std::ranges;

TEST(TSPCConvert, patchsizes)
{
    const auto patchsizes_ref = cv::imread("patchsizes@ref.tiff", cv::IMREAD_UNCHANGED);
    const auto patchsizes = cv::imread("patchsizes.tiff", cv::IMREAD_UNCHANGED);

    for (const int row : rgs::views::iota(0, patchsizes_ref.rows)) {
        auto prow_ref = patchsizes_ref.ptr<int>(row);
        auto prow = patchsizes.ptr<int>(row);
        for (const int col : rgs::views::iota(0, patchsizes_ref.cols)) {
            EXPECT_EQ(prow_ref[col], prow[col]);
        }
    }
}
