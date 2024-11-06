#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct {

namespace _io::yuv {

namespace fs = std::filesystem;

template <typename TElem_, size_t Ushift_, size_t Vshift_>
class YuvFrame_
{
public:
    static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

    using TElem = TElem_;
    static constexpr size_t Ushift = Ushift_;
    static constexpr size_t Vshift = Vshift_;

    TLCT_API inline YuvFrame_(size_t ywidth, size_t yheight, size_t ysize)
        : ywidth_(ywidth), yheight_(yheight), ysize_(ysize)
    {
        this->alloc();
    }
    TLCT_API inline YuvFrame_(size_t ywidth, size_t yheight)
        : ywidth_(ywidth), yheight_(yheight), ysize_(ywidth_ * yheight_)
    {
        this->alloc();
    }
    TLCT_API explicit inline YuvFrame_(const cv::Size& size)
        : ywidth_(size.width), yheight_(size.height), ysize_(ywidth_ * yheight_)
    {
        this->alloc();
    }

    YuvFrame_(const YuvFrame_& rhs) = delete;
    YuvFrame_ operator=(const YuvFrame_& rhs) = delete;
    TLCT_API inline YuvFrame_(YuvFrame_&& rhs) noexcept
        : ywidth_(rhs.ywidth_), yheight_(rhs.yheight_), ysize_(rhs.ysize_),
          buffer_(std::exchange(rhs.buffer_, nullptr)), y_(std::move(rhs.y_)), u_(std::move(rhs.u_)),
          v_(std::move(rhs.v_)){};
    TLCT_API inline YuvFrame_& operator=(YuvFrame_&& rhs) noexcept
    {
        ywidth_ = rhs.ywidth_;
        yheight_ = rhs.yheight_;
        ysize_ = rhs.ysize_;
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        y_ = std::move(rhs.y_);
        u_ = std::move(rhs.u_);
        v_ = std::move(rhs.v_);
        return *this;
    };

    TLCT_API inline ~YuvFrame_() { std::free(buffer_); }

    [[nodiscard]] TLCT_API inline size_t getYWidth() const noexcept { return ywidth_; };
    [[nodiscard]] TLCT_API inline size_t getYHeight() const noexcept { return yheight_; };
    [[nodiscard]] TLCT_API inline size_t getUWidth() const noexcept { return ywidth_ >> Ushift; };
    [[nodiscard]] TLCT_API inline size_t getUHeight() const noexcept { return yheight_ >> Ushift; };
    [[nodiscard]] TLCT_API inline size_t getVWidth() const noexcept { return ywidth_ >> Vshift; };
    [[nodiscard]] TLCT_API inline size_t getVHeight() const noexcept { return yheight_ >> Vshift; };
    [[nodiscard]] TLCT_API inline size_t getYSize() const noexcept { return ysize_; };
    [[nodiscard]] TLCT_API inline size_t getUSize() const noexcept { return ysize_ >> (Ushift * 2); };
    [[nodiscard]] TLCT_API inline size_t getVSize() const noexcept { return ysize_ >> (Vshift * 2); };
    [[nodiscard]] TLCT_API inline size_t getTotalSize() const noexcept
    {
        const size_t total_size = ysize_ + getUSize() + getVSize();
        return total_size;
    };

    [[nodiscard]] TLCT_API inline const cv::Mat& getY() const noexcept { return y_; }
    [[nodiscard]] TLCT_API inline const cv::Mat& getU() const noexcept { return u_; }
    [[nodiscard]] TLCT_API inline const cv::Mat& getV() const noexcept { return v_; }
    [[nodiscard]] TLCT_API inline cv::Mat& getY() noexcept { return y_; }
    [[nodiscard]] TLCT_API inline cv::Mat& getU() noexcept { return u_; }
    [[nodiscard]] TLCT_API inline cv::Mat& getV() noexcept { return v_; }

private:
    inline void alloc();

    size_t ywidth_;
    size_t yheight_;
    size_t ysize_;
    void* buffer_;
    cv::Mat y_;
    cv::Mat u_;
    cv::Mat v_;
};

template <typename TElem, size_t Ushift_, size_t Vshift_>
void YuvFrame_<TElem, Ushift_, Vshift_>::alloc()
{
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

        size_t aligned_ysize = _hp::alignUp<SIMD_FETCH_SIZE>(getYSize());
        size_t aligned_usize = _hp::alignUp<SIMD_FETCH_SIZE>(getUSize());
        size_t aligned_vsize;
        if constexpr (Ushift == Vshift) {
            aligned_vsize = aligned_usize;
        } else {
            aligned_vsize = _hp::alignUp<SIMD_FETCH_SIZE>(getVSize());
        }

        const size_t total_size = aligned_ysize + aligned_usize + aligned_vsize + SIMD_FETCH_SIZE;
        buffer_ = std::malloc(total_size);

        auto* yptr = (TElem*)_hp::roundTo<SIMD_FETCH_SIZE>((size_t)buffer_);
        auto* uptr = (TElem*)((size_t)yptr + aligned_ysize);
        auto* vptr = (TElem*)((size_t)uptr + aligned_usize);

        y_ = cv::Mat((int)getYHeight(), (int)getYWidth(), cv::DataType<TElem>::type, (void*)yptr);
        u_ = cv::Mat((int)getUHeight(), (int)getUWidth(), cv::DataType<TElem>::type, (void*)uptr);
        v_ = cv::Mat((int)getVHeight(), (int)getVWidth(), cv::DataType<TElem>::type, (void*)vptr);
    }
}

using Yuv420pFrame = YuvFrame_<uint8_t, 1, 1>;
template class YuvFrame_<uint8_t, 1, 1>;

} // namespace _io::yuv

namespace io::yuv {

namespace _ = _io::yuv;

using _::Yuv420pFrame;

} // namespace io::yuv

} // namespace tlct
