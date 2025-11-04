#include <expected>
#include <format>

#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/extent.hpp"
#endif

namespace tlct::_io {

YuvPlanarExtent::YuvPlanarExtent(int yWidth, int yHeight, int depth, int uShift, int vShift, int ySize) noexcept
    : yWidth_(yWidth), yHeight_(yHeight), depth_(depth), uShift_(uShift), vShift_(vShift), yByteSize_(ySize) {}

std::expected<YuvPlanarExtent, Error> YuvPlanarExtent::create(int yWidth, int yHeight, int depth, int uShift,
                                                              int vShift) noexcept {
    if (!_hp::isMulOf(yWidth, uShift)) [[unlikely]] {
        auto errMsg = std::format("yWidth={} must be divisible by uDivisor={}", yWidth, 1 << uShift);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (!_hp::isMulOf(yWidth, vShift)) [[unlikely]] {
        auto errMsg = std::format("yWidth={} must be divisible by vDivisor={}", yWidth, 1 << vShift);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    const int ySize = yWidth * yHeight * depth;
    return YuvPlanarExtent{yWidth, yHeight, depth, uShift, vShift, ySize};
}
std::expected<YuvPlanarExtent, Error> YuvPlanarExtent::createYuv420p8bit(int yWidth, int yHeight) noexcept {
    return create(yWidth, yHeight, 1, 1, 1);
}

}  // namespace tlct::_io
