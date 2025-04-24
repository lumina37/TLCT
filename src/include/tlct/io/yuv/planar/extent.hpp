#pragma once

#include <expected>

#include "tlct/common/defines.h"
#include "tlct/helper/error.hpp"

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
    [[nodiscard]] TLCT_API int getYSize() const noexcept { return ySize_; };
    [[nodiscard]] TLCT_API int getUWidth() const noexcept { return yWidth_ >> uShift_; }
    [[nodiscard]] TLCT_API int getUHeight() const noexcept { return yHeight_ >> uShift_; }
    [[nodiscard]] TLCT_API int getUSize() const noexcept { return ySize_ >> (uShift_ << 1); }
    [[nodiscard]] TLCT_API int getVWidth() const noexcept { return yWidth_ >> vShift_; }
    [[nodiscard]] TLCT_API int getVHeight() const noexcept { return yHeight_ >> vShift_; }
    [[nodiscard]] TLCT_API int getVSize() const noexcept { return ySize_ >> (vShift_ << 1); }
    [[nodiscard]] TLCT_API int getDepth() const noexcept { return depth_; }
    [[nodiscard]] TLCT_API int getUShift() const noexcept { return uShift_; }
    [[nodiscard]] TLCT_API int getVShift() const noexcept { return vShift_; }
    [[nodiscard]] TLCT_API int getTotalSize() const noexcept {
        const int totalSize = ySize_ + getUSize() + getVSize();
        return totalSize;
    }

private:
    int yWidth_;
    int yHeight_;
    int depth_;
    int uShift_;
    int vShift_;
    int ySize_;
};

}  // namespace tlct::_io

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/extent.cpp"
#endif
