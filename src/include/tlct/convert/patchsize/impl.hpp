#pragma once

#include <expected>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/patchsize/params.hpp"
#include "tlct/helper/error.hpp"

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange_>
class PsizeImpl_ {
public:
    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

private:
    PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& params) noexcept;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] float metricOfPsize(const TNeighbors& neighbors, const MIBuffer& anchorMI, float psize) const;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] PsizeMetric estimateWithNeighbor(const TNeighbors& neighbors, const MIBuffer& anchorMI) const;

    [[nodiscard]] float estimatePatchsize(cv::Point index) const;

    void adjustWgtsAndPsizesForMultiFocus();

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
    [[nodiscard]] TLCT_API float getPatchsize(cv::Point index) const noexcept { return patchsizes_.at<float>(index); }
    [[nodiscard]] TLCT_API float getPatchsize(int row, int col) const noexcept { return getPatchsize({col, row}); }
    [[nodiscard]] TLCT_API float getWeight(cv::Point index) const noexcept { return weights_.at<float>(index); }
    [[nodiscard]] TLCT_API float getWeight(int row, int col) const noexcept { return getWeight({col, row}); }

    // Temp helper
    [[nodiscard]] TLCT_API cv::Mat& getPatchsizes() noexcept { return patchsizes_; }
    [[nodiscard]] TLCT_API const TMIBuffers& getMIs() const noexcept { return mis_; }

    // Non-const methods
    [[nodiscard]] TLCT_API std::expected<void, Error> update(const cv::Mat& src) noexcept;

private:
    TArrange arrange_;
    TMIBuffers mis_;
    cv::Mat prevPatchsizes_;  // CV_32FC1
    cv::Mat patchsizes_;      // CV_32FC1
    cv::Mat weights_;         // CV_32FC1
    PsizeParams params_;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/impl.cpp"
#endif
