#pragma once

#include <cstddef>
#include <cstdlib>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

struct MIBuffer {
    static constexpr int C1_COUNT = 1;
    cv::Mat srcY;  // 8UC1
    static constexpr int C3_COUNT = 2;
    cv::Mat censusMap;   // 8UC3
    cv::Mat censusMask;  // 8UC3

    float intensity;
};

template <tlct::cfg::concepts::CArrange TArrange_>
class MIBuffers_ {
public:
    // Typename alias
    using TArrange = TArrange_;

    struct Params {
        static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

        Params() = default;
        explicit Params(const TArrange& arrange) noexcept;
        Params& operator=(Params&& rhs) noexcept = default;
        Params(Params&& rhs) noexcept = default;

        size_t alignedMatSizeC1_;
        size_t alignedMatSizeC3_;
        size_t alignedMISize_;
        size_t bufferSize_;
        int iDiameter_;
        int miMaxCols_;
        int miNum_;
    };

    // Constructor
    MIBuffers_() noexcept : arrange_(), params_(), miBuffers_(), buffer_(nullptr) {}
    explicit MIBuffers_(const TArrange& arrange);
    MIBuffers_& operator=(const MIBuffers_& rhs) = delete;
    MIBuffers_(const MIBuffers_& rhs) = delete;
    MIBuffers_& operator=(MIBuffers_&& rhs) noexcept {
        arrange_ = std::move(rhs.arrange_);
        params_ = std::move(rhs.params_);
        miBuffers_ = std::move(rhs.miBuffers_);
        buffer_ = std::exchange(rhs.buffer_, nullptr);
        return *this;
    }
    MIBuffers_(MIBuffers_&& rhs) noexcept
        : arrange_(std::move(rhs.arrange_)),
          params_(std::move(rhs.params_)),
          miBuffers_(std::move(rhs.miBuffers_)),
          buffer_(std::exchange(rhs.buffer_, nullptr)) {}
    ~MIBuffers_() { std::free(buffer_); }

    // Initialize from
    [[nodiscard]] static MIBuffers_ fromArrange(const TArrange& arrange);

    // Const methods
    [[nodiscard]] const MIBuffer& getMI(const int row, const int col) const noexcept {
        const int offset = row * params_.miMaxCols_ + col;
        return miBuffers_.at(offset);
    }
    [[nodiscard]] const MIBuffer& getMI(const cv::Point index) const noexcept { return getMI(index.y, index.x); }
    [[nodiscard]] const MIBuffer& getMI(const int offset) const noexcept { return miBuffers_.at(offset); }

    // Non-const methods
    MIBuffers_& update(const cv::Mat& src);

private:
    TArrange arrange_;
    Params params_;
    std::vector<MIBuffer> miBuffers_;
    void* buffer_;
};

[[nodiscard]] float censusCompare(const MIBuffer& lhsMI, const MIBuffer& rhsMI, cv::Point2f offset) noexcept;

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/mibuffer.cpp"
#endif
