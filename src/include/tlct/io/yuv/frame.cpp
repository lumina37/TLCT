#include <cstdlib>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/helper/constexpr/math.hpp"
#include "tlct/io/concepts/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/frame.hpp"
#endif

namespace tlct::_io::yuv {

template <typename TElem, size_t Ushift_, size_t Vshift_>
void YuvFrame_<TElem, Ushift_, Vshift_>::alloc() {
    {
        constexpr size_t ubase = 1 << (Ushift * 2);

        if (!_hp::isMulOf<ubase>(getYSize())) {
            return;
        }

        if constexpr (Ushift != Vshift) {
            constexpr size_t vbase = 1 << (Vshift * 2);
            if (!_hp::isMulOf<vbase>(getYSize())) {
                return;
            }
        }

        size_t alignedYSize = _hp::alignUp<SIMD_FETCH_SIZE>(getYSize());
        size_t alignedUSize = _hp::alignUp<SIMD_FETCH_SIZE>(getUSize());
        size_t alignedVSize;
        if constexpr (Ushift == Vshift) {
            alignedVSize = alignedUSize;
        } else {
            alignedVSize = _hp::alignUp<SIMD_FETCH_SIZE>(getVSize());
        }

        const size_t totalSize = alignedYSize + alignedUSize + alignedVSize;
        buffer_ = std::malloc(totalSize);

        TElem* yptr = (TElem*)buffer_;
        TElem* uptr = (TElem*)((size_t)yptr + alignedYSize);
        TElem* vptr = (TElem*)((size_t)uptr + alignedUSize);

        y_ = cv::Mat((int)getYHeight(), (int)getYWidth(), cv::DataType<TElem>::type, (void*)yptr);
        u_ = cv::Mat((int)getUHeight(), (int)getUWidth(), cv::DataType<TElem>::type, (void*)uptr);
        v_ = cv::Mat((int)getVHeight(), (int)getVWidth(), cv::DataType<TElem>::type, (void*)vptr);
    }
}

static_assert(concepts::CFrame<Yuv420Frame>);
template class YuvFrame_<uint8_t, 1, 1>;

}  // namespace tlct::_io::yuv
