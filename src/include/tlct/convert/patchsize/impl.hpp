#pragma once

#include <expected>
#include <filesystem>
#include <vector>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/params.hpp"
#include "tlct/convert/patchsize/record.hpp"
#include "tlct/helper/error.hpp"

namespace tlct::_cvt {

namespace fs = std::filesystem;

template <cfg::concepts::CArrange TArrange_>
class PsizeImpl_ {
public:
#ifdef _DEBUG
    static constexpr bool ENABLE_DEBUG = true;
#else
    static constexpr bool ENABLE_DEBUG = false;
#endif

    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;
    using PatchRecord = PatchRecord_<ENABLE_DEBUG>;

private:
    PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& params) noexcept;

    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;

    [[nodiscard]] typename NearNeighbors::Direction maxGradDirectionWithNearNeighbors(
        const NearNeighbors& neighbors) const noexcept;
    [[nodiscard]] typename FarNeighbors::Direction maxGradDirectionWithFarNeighbors(
        const FarNeighbors& neighbors) const noexcept;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] PsizeMetric estimateWithNeighbors(const TNeighbors& neighbors, const MIBuffer& anchorMI,
                                                    typename TNeighbors::Direction direction) noexcept;

    [[nodiscard]] float estimatePatchsize(cv::Point index) noexcept;

    void adjustWgtsAndPsizesForMultiFocus() noexcept;

public:
    // Constructor
    PsizeImpl_() = delete;
    PsizeImpl_(const PsizeImpl_& rhs) = delete;
    PsizeImpl_& operator=(const PsizeImpl_& rhs) = delete;
    PsizeImpl_(PsizeImpl_&& rhs) noexcept = default;
    PsizeImpl_& operator=(PsizeImpl_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PsizeImpl_, Error> create(const TArrange& arrange,
                                                                          const TCvtConfig& cvtCfg) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API float getPatchsize(int offset) const noexcept { return patchRecords_[offset].getPsize(); }
    [[nodiscard]] TLCT_API float getPatchsize(cv::Point index) const noexcept { return getPatchsize(index.y, index.x); }
    [[nodiscard]] TLCT_API float getPatchsize(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getPatchsize(offset);
    }

    [[nodiscard]] TLCT_API float getWeight(int offset) const noexcept { return weights_[offset]; }
    [[nodiscard]] TLCT_API float getWeight(cv::Point index) const noexcept { return getWeight(index.y, index.x); }
    [[nodiscard]] TLCT_API float getWeight(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getWeight(offset);
    }

    [[nodiscard]] TLCT_API std::expected<void, Error> dumpRecords(const fs::path& dumpTo) const noexcept;

    // Non-const methods
    [[nodiscard]] TLCT_API std::expected<void, Error> update(const cv::Mat& src) noexcept;

    [[nodiscard]] TLCT_API std::expected<void, Error> loadRecords(const fs::path& loadFrom) noexcept;

private:
    [[nodiscard]] float getPrevPatchsize(int offset) const noexcept { return prevPatchRecords_[offset].getPsize(); }
    [[nodiscard]] float getPrevPatchsize(cv::Point index) const noexcept { return getPrevPatchsize(index.y, index.x); }
    [[nodiscard]] float getPrevPatchsize(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getPrevPatchsize(offset);
    }

    TArrange arrange_;
    TMIBuffers mis_;
    std::vector<PatchRecord> prevPatchRecords_;
    std::vector<PatchRecord> patchRecords_;
    std::vector<float> weights_;
    PsizeParams params_;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/impl.cpp"
#endif
