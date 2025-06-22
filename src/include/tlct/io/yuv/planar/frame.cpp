#include <cstddef>
#include <expected>
#include <format>
#include <new>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io/yuv/planar/extent.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/frame.hpp"
#endif

namespace tlct::_io {

YuvPlanarFrame::YuvPlanarFrame(const YuvPlanarExtent& extent, std::unique_ptr<std::byte[]>&& pBuffer, cv::Mat&& y,
                               cv::Mat&& u, cv::Mat&& v) noexcept
    : extent_(extent), pBuffer_(std::move(pBuffer)), y_(std::move(y)), u_(std::move(u)), v_(std::move(v)) {}

std::expected<YuvPlanarFrame, Error> YuvPlanarFrame::create(const YuvPlanarExtent& extent) noexcept {
    int cvTp;
    switch (extent.getDepth()) {
        case 1:
            cvTp = CV_8U;
            break;
        case 2:
            cvTp = CV_16U;
            break;
        default:
            [[unlikely]] {
                auto errMsg = std::format("expect depth 1(8bit) or 2(16bit), got {}", extent.getDepth());
                return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
            }
    }

    size_t alignedYSize = _hp::alignUp<SIMD_FETCH_SIZE>(extent.getYSize());
    size_t alignedUSize = _hp::alignUp<SIMD_FETCH_SIZE>(extent.getUSize());
    size_t alignedVSize = _hp::alignUp<SIMD_FETCH_SIZE>(extent.getVSize());

    const size_t totalSize = alignedYSize + alignedUSize + alignedVSize;
    std::unique_ptr<std::byte[]> pBuffer;
    try {
        pBuffer = std::make_unique_for_overwrite<std::byte[]>(totalSize);
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ErrCode::OutOfMemory}};
    }

    std::byte* yptr = pBuffer.get();
    std::byte* uptr = (std::byte*)((size_t)yptr + alignedYSize);
    std::byte* vptr = (std::byte*)((size_t)uptr + alignedUSize);

    cv::Mat y = cv::Mat(extent.getYHeight(), extent.getYWidth(), cvTp, yptr);
    cv::Mat u = cv::Mat(extent.getUHeight(), extent.getUWidth(), cvTp, uptr);
    cv::Mat v = cv::Mat(extent.getVHeight(), extent.getVWidth(), cvTp, vptr);

    return YuvPlanarFrame{extent, std::move(pBuffer), std::move(y), std::move(u), std::move(v)};
}

}  // namespace tlct::_io
