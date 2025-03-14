#pragma once

#include <cstdlib>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct::_io::yuv {

template <typename TElem_, size_t Ushift_, size_t Vshift_>
class YuvFrame_ {
public:
    static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

    using TElem = TElem_;
    static constexpr size_t Ushift = Ushift_;
    static constexpr size_t Vshift = Vshift_;

    TLCT_API YuvFrame_(size_t yWidth, size_t yHeight, size_t ySize)
        : yWidth_(yWidth), yHeight_(yHeight), ySize_(ySize) {
        this->alloc();
    }
    TLCT_API YuvFrame_(size_t yWidth, size_t yHeight) : yWidth_(yWidth), yHeight_(yHeight), ySize_(yWidth_ * yHeight_) {
        this->alloc();
    }
    TLCT_API explicit YuvFrame_(const cv::Size& size)
        : yWidth_(size.width), yHeight_(size.height), ySize_(yWidth_ * yHeight_) {
        this->alloc();
    }

    YuvFrame_() = delete;
    YuvFrame_(const YuvFrame_& rhs) = delete;
    YuvFrame_ operator=(const YuvFrame_& rhs) = delete;
    TLCT_API YuvFrame_(YuvFrame_&& rhs) noexcept
        : yWidth_(rhs.yWidth_),
          yHeight_(rhs.yHeight_),
          ySize_(rhs.ySize_),
          buffer_(std::exchange(rhs.buffer_, nullptr)),
          y_(std::move(rhs.y_)),
          u_(std::move(rhs.u_)),
          v_(std::move(rhs.v_)) {};
    TLCT_API YuvFrame_& operator=(YuvFrame_&& rhs) noexcept {
        yWidth_ = rhs.yWidth_;
        yHeight_ = rhs.yHeight_;
        ySize_ = rhs.ySize_;
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        y_ = std::move(rhs.y_);
        u_ = std::move(rhs.u_);
        v_ = std::move(rhs.v_);
        return *this;
    };

    TLCT_API ~YuvFrame_() { std::free(buffer_); }

    [[nodiscard]] TLCT_API size_t getYWidth() const noexcept { return yWidth_; };
    [[nodiscard]] TLCT_API size_t getYHeight() const noexcept { return yHeight_; };
    [[nodiscard]] TLCT_API size_t getUWidth() const noexcept { return yWidth_ >> Ushift; };
    [[nodiscard]] TLCT_API size_t getUHeight() const noexcept { return yHeight_ >> Ushift; };
    [[nodiscard]] TLCT_API size_t getVWidth() const noexcept { return yWidth_ >> Vshift; };
    [[nodiscard]] TLCT_API size_t getVHeight() const noexcept { return yHeight_ >> Vshift; };
    [[nodiscard]] TLCT_API size_t getYSize() const noexcept { return ySize_; };
    [[nodiscard]] TLCT_API size_t getUSize() const noexcept { return ySize_ >> (Ushift * 2); };
    [[nodiscard]] TLCT_API size_t getVSize() const noexcept { return ySize_ >> (Vshift * 2); };
    [[nodiscard]] TLCT_API size_t getTotalSize() const noexcept {
        const size_t totalSize = ySize_ + getUSize() + getVSize();
        return totalSize;
    };

    [[nodiscard]] TLCT_API const cv::Mat& getY() const noexcept { return y_; }
    [[nodiscard]] TLCT_API const cv::Mat& getU() const noexcept { return u_; }
    [[nodiscard]] TLCT_API const cv::Mat& getV() const noexcept { return v_; }

    [[nodiscard]] TLCT_API cv::Mat& getY() noexcept { return y_; }
    [[nodiscard]] TLCT_API cv::Mat& getU() noexcept { return u_; }
    [[nodiscard]] TLCT_API cv::Mat& getV() noexcept { return v_; }

private:
    void alloc();

    size_t yWidth_;
    size_t yHeight_;
    size_t ySize_;
    void* buffer_;
    cv::Mat y_;
    cv::Mat u_;
    cv::Mat v_;
};

using Yuv420Frame = YuvFrame_<uint8_t, 1, 1>;

}  // namespace tlct::_io::yuv

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/frame.cpp"
#endif
