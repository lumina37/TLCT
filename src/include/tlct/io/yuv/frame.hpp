#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_io::yuv {

namespace fs = std::filesystem;

template <typename TElem_, size_t Ushift_, size_t Vshift_>
class YuvFrame_ {
public:
    static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

    using TElem = TElem_;
    static constexpr size_t Ushift = Ushift_;
    static constexpr size_t Vshift = Vshift_;

    TLCT_API inline YuvFrame_(size_t yWidth, size_t yHeight, size_t ySize)
        : yWidth_(yWidth), yHeight_(yHeight), ySize_(ySize) {
        this->alloc();
    }
    TLCT_API inline YuvFrame_(size_t yWidth, size_t yHeight)
        : yWidth_(yWidth), yHeight_(yHeight), ySize_(yWidth_ * yHeight_) {
        this->alloc();
    }
    TLCT_API explicit inline YuvFrame_(const cv::Size& size)
        : yWidth_(size.width), yHeight_(size.height), ySize_(yWidth_ * yHeight_) {
        this->alloc();
    }

    YuvFrame_() = delete;
    YuvFrame_(const YuvFrame_& rhs) = delete;
    YuvFrame_ operator=(const YuvFrame_& rhs) = delete;
    TLCT_API inline YuvFrame_(YuvFrame_&& rhs) noexcept
        : yWidth_(rhs.yWidth_),
          yHeight_(rhs.yHeight_),
          ySize_(rhs.ySize_),
          buffer_(std::exchange(rhs.buffer_, nullptr)),
          y_(std::move(rhs.y_)),
          u_(std::move(rhs.u_)),
          v_(std::move(rhs.v_)){};
    TLCT_API inline YuvFrame_& operator=(YuvFrame_&& rhs) noexcept {
        yWidth_ = rhs.yWidth_;
        yHeight_ = rhs.yHeight_;
        ySize_ = rhs.ySize_;
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        y_ = std::move(rhs.y_);
        u_ = std::move(rhs.u_);
        v_ = std::move(rhs.v_);
        return *this;
    };

    TLCT_API inline ~YuvFrame_() { std::free(buffer_); }

    [[nodiscard]] TLCT_API inline size_t getYWidth() const noexcept { return yWidth_; };
    [[nodiscard]] TLCT_API inline size_t getYHeight() const noexcept { return yHeight_; };
    [[nodiscard]] TLCT_API inline size_t getUWidth() const noexcept { return yWidth_ >> Ushift; };
    [[nodiscard]] TLCT_API inline size_t getUHeight() const noexcept { return yHeight_ >> Ushift; };
    [[nodiscard]] TLCT_API inline size_t getVWidth() const noexcept { return yWidth_ >> Vshift; };
    [[nodiscard]] TLCT_API inline size_t getVHeight() const noexcept { return yHeight_ >> Vshift; };
    [[nodiscard]] TLCT_API inline size_t getYSize() const noexcept { return ySize_; };
    [[nodiscard]] TLCT_API inline size_t getUSize() const noexcept { return ySize_ >> (Ushift * 2); };
    [[nodiscard]] TLCT_API inline size_t getVSize() const noexcept { return ySize_ >> (Vshift * 2); };
    [[nodiscard]] TLCT_API inline size_t getTotalSize() const noexcept {
        const size_t totalSize = ySize_ + getUSize() + getVSize();
        return totalSize;
    };

    [[nodiscard]] TLCT_API inline const cv::Mat& getY() const noexcept { return y_; }
    [[nodiscard]] TLCT_API inline const cv::Mat& getU() const noexcept { return u_; }
    [[nodiscard]] TLCT_API inline const cv::Mat& getV() const noexcept { return v_; }

    [[nodiscard]] TLCT_API inline cv::Mat& getY() noexcept { return y_; }
    [[nodiscard]] TLCT_API inline cv::Mat& getU() noexcept { return u_; }
    [[nodiscard]] TLCT_API inline cv::Mat& getV() noexcept { return v_; }

private:
    inline void alloc();

    size_t yWidth_;
    size_t yHeight_;
    size_t ySize_;
    void* buffer_;
    cv::Mat y_;
    cv::Mat u_;
    cv::Mat v_;
};

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

}  // namespace tlct::_io::yuv
