#pragma once

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/common/bridge/patch_merge.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/patchsize/helper/neighbors.hpp"
#include "tlct/convert/patchsize/ssim/functional.hpp"
#include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#include "tlct/convert/patchsize/ssim/params.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::dbg {

struct PsizeMetric {
    float psize;
    float metric;
};

template <cfg::concepts::CArrange TArrange_>
class PsizeImpl_ {
public:
    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = cfg::CliConfig::Convert;
    using TBridge = PatchMergeBridge_<TArrange>;

private:
    using TMIBuffers = ssim::MIBuffers_<TArrange>;
    using TPsizeParams = ssim::PsizeParams_<TArrange>;
    using TPInfo = TBridge::TInfo;
    using TPInfos = TBridge::TInfos;

    PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TMIBuffers&& prevMis, TPInfos&& prevPatchInfos,
               const TPsizeParams& params) noexcept;

    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] PsizeMetric estimateWithNeighbors(const TNeighbors& neighbors,
                                                    ssim::WrapSSIM& wrapAnchor) const noexcept;

    [[nodiscard]] float estimatePatchsize(TBridge& bridge, cv::Point index) const noexcept;

    void adjustWgtsAndPsizesForMultiFocus(TBridge& bridge) noexcept;

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

    // Non-const methods
    [[nodiscard]] TLCT_API std::expected<void, Error> updateBridge(const cv::Mat& src, TBridge& bridge) noexcept;

private:
    [[nodiscard]] float getPrevPatchsize(int offset) const noexcept { return prevPatchInfos_[offset].getPatchsize(); }

    TArrange arrange_;
    TMIBuffers mis_;
    TMIBuffers prevMis_;
    TPInfos prevPatchInfos_;
    TPsizeParams params_;
};

}  // namespace tlct::_cvt::dbg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/debug/impl.cpp"
#endif
