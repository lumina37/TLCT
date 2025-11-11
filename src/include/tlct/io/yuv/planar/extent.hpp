#pragma once

#include <opencv2/core.hpp>

#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_io {

class YuvPlanarExtent {
    YuvPlanarExtent(int yWidth, int yHeight, int depth, int uShift, int vShift, int ySize) noexcept;

public:
    [[nodiscard]] TLCT_API static std::expected<YuvPlanarExtent, Error> create(int yWidth, int yHeight, int depth,
                                                                               int uShift, int vShift) noexcept;
    [[nodiscard]] TLCT_API static std::expected<YuvPlanarExtent, Error> createYuv420p8bit(int yWidth,
                                                                                          int yHeight) noexcept;

    [[nodiscard]] TLCT_API int getYWidth() const noexcept { return yWidth_; }
    [[nodiscard]] TLCT_API int getYHeight() const noexcept { return yHeight_; }
    [[nodiscard]] TLCT_API cv::Size getYSize() const noexcept { return {getYWidth(), getYHeight()}; }
    [[nodiscard]] TLCT_API int getYByteSize() const noexcept { return yByteSize_; }
    [[nodiscard]] TLCT_API int getUWidth() const noexcept { return yWidth_ >> uShift_; }
    [[nodiscard]] TLCT_API int getUHeight() const noexcept { return yHeight_ >> uShift_; }
    [[nodiscard]] TLCT_API cv::Size getUSize() const noexcept { return {getUWidth(), getUHeight()}; }
    [[nodiscard]] TLCT_API int getUByteSize() const noexcept { return yByteSize_ >> (uShift_ << 1); }
    [[nodiscard]] TLCT_API int getVWidth() const noexcept { return yWidth_ >> vShift_; }
    [[nodiscard]] TLCT_API int getVHeight() const noexcept { return yHeight_ >> vShift_; }
    [[nodiscard]] TLCT_API cv::Size getVSize() const noexcept { return {getVWidth(), getVHeight()}; }
    [[nodiscard]] TLCT_API int getVByteSize() const noexcept { return yByteSize_ >> (vShift_ << 1); }
    [[nodiscard]] TLCT_API int getDepth() const noexcept { return depth_; }
    [[nodiscard]] TLCT_API int getUShift() const noexcept { return uShift_; }
    [[nodiscard]] TLCT_API int getVShift() const noexcept { return vShift_; }
    [[nodiscard]] TLCT_API int getTotalByteSize() const noexcept {
        const int totalByteSize = getYByteSize() + getUByteSize() + getVByteSize();
        return totalByteSize;
    }

private:
    int yWidth_;
    int yHeight_;
    int depth_;
    int uShift_;
    int vShift_;
    int yByteSize_;
};

}  // namespace tlct::_io

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/extent.cpp"
#endif
