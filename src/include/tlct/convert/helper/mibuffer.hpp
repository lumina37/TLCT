#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ranges>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/helper/constexpr/math.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

struct MIBuffer {
    cv::Mat srcY;        // 8UC1
    cv::Mat censusMap;   // 8UC3
    cv::Mat censusMask;  // 8UC3
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
            alignedMatSizeC1_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_);
            alignedMatSizeC3_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_ * 3);
            alignedMISize_ = alignedMatSizeC3_ * 2 + alignedMatSizeC1_;
            miMaxCols_ = arrange.getMIMaxCols();
            miNum_ = miMaxCols_ * arrange.getMIRows();
            bufferSize_ = miNum_ * alignedMISize_;
        };
        inline Params& operator=(Params&& rhs) noexcept = default;
        inline Params(Params&& rhs) noexcept = default;

        size_t alignedMatSizeC1_;
        size_t alignedMatSizeC3_;
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
    int iDiameter = _hp::iround(arrange_.getDiameter());
    int iRadius = _hp::iround(arrange_.getRadius());
    cv::Mat srcMask = cv::Mat::zeros(iDiameter, iDiameter, CV_8UC1);
    cv::circle(srcMask, {iRadius, iRadius}, iRadius, cv::Scalar::all(0xff), cv::FILLED);

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

            const cv::Mat& srcY = src(roi);
            cv::Mat dstI = cv::Mat(params_.idiameter_, params_.idiameter_, CV_8UC1, matCursor);
            srcY.copyTo(dstI);
            matCursor += params_.alignedMatSizeC1_;

            cv::Mat censusMap = cv::Mat(params_.idiameter_, params_.idiameter_, CV_8UC3, matCursor);
            matCursor += params_.alignedMatSizeC3_;
            cv::Mat censusMask = cv::Mat(params_.idiameter_, params_.idiameter_, CV_8UC3, matCursor);
            matCursor += params_.alignedMatSizeC3_;
            censusTransform5x5(srcY, srcMask, censusMap, censusMask);

            *itemIt = {std::move(dstI), std::move(censusMap), std::move(censusMask)};
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
