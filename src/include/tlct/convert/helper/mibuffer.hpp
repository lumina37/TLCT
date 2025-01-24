#pragma once

#include <cstddef>
#include <ranges>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

struct MIBuffer {
    cv::Mat I, I_2;
};

template <tlct::cfg::concepts::CArrange TArrange_>
class MIBuffers_ {
public:
    // Typename alias
    using TArrange = TArrange_;

    struct Params {
        static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

        inline Params() = default;
        inline explicit Params(const TArrange& arrange) noexcept {
            idiameter_ = _hp::iround(arrange.getDiameter());
            alignedMatSize_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_ * sizeof(float));
            alignedMISize_ = (sizeof(MIBuffer) / sizeof(cv::Mat)) * alignedMatSize_;
            miMaxCols_ = arrange.getMIMaxCols();
            miNum_ = miMaxCols_ * arrange.getMIRows();
            bufferSize_ = miNum_ * alignedMISize_;
        };
        inline Params& operator=(Params&& rhs) noexcept = default;
        inline Params(Params&& rhs) noexcept = default;

        size_t alignedMatSize_;
        size_t alignedMISize_;
        size_t bufferSize_;
        int idiameter_;
        int miMaxCols_;
        int miNum_;
    };

    // Constructor
    inline MIBuffers_() noexcept : arrange_(), params_(), items_(), buffer_(nullptr) {};
    inline explicit MIBuffers_(const TArrange& arrange);
    MIBuffers_& operator=(const MIBuffers_& rhs) = delete;
    MIBuffers_(const MIBuffers_& rhs) = delete;
    inline MIBuffers_& operator=(MIBuffers_&& rhs) noexcept {
        arrange_ = std::move(rhs.arrange_);
        params_ = std::move(rhs.params_);
        items_ = std::move(rhs.items_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    };
    inline MIBuffers_(MIBuffers_&& rhs) noexcept
        : arrange_(std::move(rhs.arrange_)),
          params_(std::move(rhs.params_)),
          items_(std::move(rhs.items_)),
          buffer_(std::exchange(rhs.buffer_, nullptr)) {};
    inline ~MIBuffers_() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] static inline MIBuffers_ fromArrange(const TArrange& arrange);

    // Const methods
    [[nodiscard]] inline const MIBuffer& getMI(int row, int col) const noexcept {
        const int offset = row * params_.miMaxCols_ + col;
        return items_.at(offset);
    };
    [[nodiscard]] inline const MIBuffer& getMI(cv::Point index) const noexcept { return getMI(index.y, index.x); };
    [[nodiscard]] inline const MIBuffer& getMI(int offset) const noexcept { return items_.at(offset); };

    // Non-const methods
    inline MIBuffers_& update(const cv::Mat& src);

private:
    TArrange arrange_;
    Params params_;
    std::vector<MIBuffer> items_;
    void* buffer_;
};

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::MIBuffers_(const TArrange& arrange) : arrange_(arrange), params_(arrange) {
    items_.resize(params_.miNum_);
    buffer_ = std::malloc(params_.bufferSize_ + Params::SIMD_FETCH_SIZE);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange> MIBuffers_<TArrange>::fromArrange(const TArrange& arrange) {
    return MIBuffers_(arrange);
}

template <tlct::cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>& MIBuffers_<TArrange>::update(const cv::Mat& src) {
    cv::Mat f32I2;
    const cv::Mat& f32I = src;
    cv::multiply(f32I, f32I, f32I2);

    auto itemIt = items_.begin();
    uint8_t* rowCursor = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)buffer_);
    size_t rowStep = params_.miMaxCols_ * params_.alignedMISize_;
    for (const int irow : rgs::views::iota(0, arrange_.getMIRows())) {
        uint8_t* colCursor = rowCursor;
        const int miCols = arrange_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, miCols)) {
            const cv::Point2f& miCenter = arrange_.getMICenter(irow, icol);
            const cv::Rect roi = getRoiByCenter(miCenter, arrange_.getDiameter());

            uint8_t* matCursor = colCursor;

            const cv::Mat& srcI = f32I(roi);
            cv::Mat dstI = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matCursor);
            srcI.copyTo(dstI);
            matCursor += params_.alignedMatSize_;

            const cv::Mat& srcI2 = f32I2(roi);
            cv::Mat dstI2 = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matCursor);
            srcI2.copyTo(dstI2);
            matCursor += params_.alignedMatSize_;

            *itemIt = {std::move(dstI), std::move(dstI2)};
            itemIt++;
            colCursor += params_.alignedMISize_;
        }

        if (miCols < params_.miMaxCols_) {
            itemIt++;
        }

        rowCursor += rowStep;
    }

    return *this;
}

}  // namespace tlct::_cvt
