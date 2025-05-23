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

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class PatchsizeImpl_ {
public:
    // Typename alias
    using TError = Error;
    using TCvtConfig = tcfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;

private:
    PatchsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, const PsizeParams& psizeParams);

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] float metricOfPsize(const TNeighbors& neighbors, const MIBuffer& anchorMI, float psize) const;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] PsizeMetric estimateWithNeighbor(const TNeighbors& neighbors, const MIBuffer& anchorMI) const;

    [[nodiscard]] float estimatePatchsize(cv::Point index) const;

public:
    // Constructor
    PatchsizeImpl_() = delete;
    PatchsizeImpl_(const PatchsizeImpl_& rhs) = delete;
    PatchsizeImpl_& operator=(const PatchsizeImpl_& rhs) = delete;
    PatchsizeImpl_(PatchsizeImpl_&& rhs) noexcept = default;
    PatchsizeImpl_& operator=(PatchsizeImpl_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PatchsizeImpl_, TError> create(const TArrange& arrange,
                                                                               const TCvtConfig& cvtCfg) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API float getPatchsize(cv::Point index) const noexcept { return patchsizes_.at<float>(index); }
    [[nodiscard]] TLCT_API float getPatchsize(int row, int col) const noexcept { return getPatchsize({col, row}); }
    [[nodiscard]] TLCT_API float getIntensity(cv::Point index) const noexcept { return mis_.getMI(index).intensity; }
    [[nodiscard]] TLCT_API float getIntensity(int row, int col) const noexcept { return getIntensity({col, row}); }

    // Temp helper
    [[nodiscard]] TLCT_API cv::Mat& getPatchsizes() noexcept { return patchsizes_; }
    [[nodiscard]] TLCT_API const TMIBuffers& getMIs() const noexcept { return mis_; }

    // Non-const methods
    [[nodiscard]] std::expected<void, Error> step(const cv::Mat& newSrc) noexcept;

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
