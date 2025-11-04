#pragma once

#include <cstddef>
#include <expected>
#include <memory>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/helper/error.hpp"
#include "tlct/io/yuv/planar/extent.hpp"

namespace tlct::_io {

class YuvPlanarFrame {
    YuvPlanarFrame(const YuvPlanarExtent& extent, std::unique_ptr<std::byte[]>&& pBuffer, cv::Mat&& y, cv::Mat&& u,
                   cv::Mat&& v) noexcept;

public:
    static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

    YuvPlanarFrame() = delete;
    YuvPlanarFrame(const YuvPlanarFrame& rhs) = delete;
    YuvPlanarFrame operator=(const YuvPlanarFrame& rhs) = delete;
    YuvPlanarFrame(YuvPlanarFrame&& rhs) noexcept = default;
    YuvPlanarFrame& operator=(YuvPlanarFrame&& rhs) noexcept = default;

    [[nodiscard]] TLCT_API static std::expected<YuvPlanarFrame, Error> create(const YuvPlanarExtent& extent) noexcept;

    [[nodiscard]] TLCT_API const YuvPlanarExtent& getExtent() const noexcept { return extent_; }
    [[nodiscard]] TLCT_API const cv::Mat& getY() const noexcept { return y_; }
    [[nodiscard]] TLCT_API const cv::Mat& getU() const noexcept { return u_; }
    [[nodiscard]] TLCT_API const cv::Mat& getV() const noexcept { return v_; }
    [[nodiscard]] TLCT_API cv::Mat& getY() noexcept { return y_; }
    [[nodiscard]] TLCT_API cv::Mat& getU() noexcept { return u_; }
    [[nodiscard]] TLCT_API cv::Mat& getV() noexcept { return v_; }

private:
    YuvPlanarExtent extent_;
    std::unique_ptr<std::byte[]> pBuffer_;
    cv::Mat y_;
    cv::Mat u_;
    cv::Mat v_;
};

}  // namespace tlct::_io

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/frame.cpp"
#endif
