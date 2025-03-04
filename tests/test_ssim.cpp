#include <filesystem>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <opencv2/imgcodecs.hpp>

#include "tlct.hpp"

#ifndef TLCT_TESTDATA_DIR
#    define TLCT_TESTDATA_DIR "."
#endif

namespace fs = std::filesystem;

TEST_CASE("SSIM", "tlct::_cvt#WrapSSIM") {
    const fs::path testdataDir{TLCT_TESTDATA_DIR};
    fs::current_path(testdataDir);

    cv::Mat u8Lhs = cv::imread("test/lhs.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat lhs, lhs_2;
    u8Lhs.convertTo(lhs, CV_32FC1);
    cv::multiply(lhs, lhs, lhs_2);

    cv::Mat u8Rhs = cv::imread("test/rhs.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat rhs, rhs_2;
    u8Rhs.convertTo(rhs, CV_32FC1);
    cv::multiply(rhs, rhs, rhs_2);

    auto lhsBuffer = tlct::_cvt::MIBuffer{lhs, lhs_2};
    auto lhsWrap = tlct::_cvt::WrapSSIM{lhsBuffer};
    lhsWrap.updateRoi(cv::Rect{0, 0, lhs.cols, lhs.rows});

    auto rhsBuffer = tlct::_cvt::MIBuffer{rhs, rhs_2};
    auto rhsWrap = tlct::_cvt::WrapSSIM{rhsBuffer};
    rhsWrap.updateRoi(cv::Rect{0, 0, rhs.cols, rhs.rows});

    auto ssim = lhsWrap.compare(rhsWrap);

    constexpr float eps = 0.0001;
    REQUIRE_THAT(ssim, Catch::Matchers::WithinAbs(0.66825, eps));
}
