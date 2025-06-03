#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <utility>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/helper/error.hpp"

namespace tlct::_cvt {

struct MIBuffer {
    static constexpr int C3_COUNT = 2;
    cv::Mat censusMap;   // 8UC3
    cv::Mat censusMask;  // 8UC3

    Grads grads;
    uint16_t dhash;
};

template <cfg::concepts::CArrange TArrange_>
class MIBuffers_ {
public:
    // Typename alias
    using TArrange = TArrange_;

    struct Params {
        static constexpr size_t SIMD_FETCH_SIZE = 128 / 8;

        Params() = default;
        Params(const TArrange& arrange) noexcept;
        Params& operator=(Params&& rhs) noexcept = default;
        Params(Params&& rhs) noexcept = default;

        size_t alignedMatSizeC3_;
        size_t alignedMISize_;
        size_t bufferSize_;
        float censusDiameter_;
        int miMaxCols_;
        int miNum_;
    };

private:
    MIBuffers_(TArrange&& arrange, Params&& params, std::vector<MIBuffer>&& miBuffers,
               std::unique_ptr<std::byte[]>&& pBuffer) noexcept;

public:
    // Constructor
    MIBuffers_() noexcept = default;
    MIBuffers_& operator=(const MIBuffers_& rhs) = delete;
    MIBuffers_(const MIBuffers_& rhs) = delete;
    MIBuffers_& operator=(MIBuffers_&& rhs) noexcept = default;
    MIBuffers_(MIBuffers_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MIBuffers_, Error> create(const TArrange& arrange) noexcept;

    // Const methods
    [[nodiscard]] const MIBuffer& getMI(const int offset) const noexcept { return miBuffers_.at(offset); }
    [[nodiscard]] const MIBuffer& getMI(const int row, const int col) const noexcept {
        const int offset = row * params_.miMaxCols_ + col;
        return getMI(offset);
    }
    [[nodiscard]] const MIBuffer& getMI(const cv::Point index) const noexcept { return getMI(index.y, index.x); }

    // Non-const methods
    [[nodiscard]] TLCT_API std::expected<void, Error> update(const cv::Mat& src) noexcept;

private:
    TArrange arrange_;
    Params params_;
    std::vector<MIBuffer> miBuffers_;
    std::unique_ptr<std::byte[]> pBuffer_;
};

[[nodiscard]] TLCT_API float compare(const MIBuffer& lhsMI, const MIBuffer& rhsMI, cv::Point2f offset) noexcept;

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/mibuffer.cpp"
#endif
