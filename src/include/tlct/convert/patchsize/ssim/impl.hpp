#pragma once

#include <filesystem>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/common/bridge/patch_merge.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/patchsize/neighbors.hpp"
#include "tlct/convert/patchsize/ssim/functional.hpp"
#include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#include "tlct/convert/patchsize/ssim/params.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::ssim {

namespace fs = std::filesystem;

struct PsizeMetric {
    float psize;
    float metric;
};

template <cfg::concepts::CArrange TArrange_>
class PsizeImpl_ {
public:
    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using TPsizeParams = PsizeParams_<TArrange>;
    using TBridge = PatchMergeBridge_<TArrange>;
    using TPInfo = TBridge::TInfo;
    using TPInfos = TBridge::TInfos;

private:
    PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TPInfos&& prevPatchInfos, TBridge&& bridge,
               const TPsizeParams& params) noexcept;

    using NearNeighbors = NearNeighbors_<TArrange>;
    using FarNeighbors = FarNeighbors_<TArrange>;

    template <concepts::CNeighbors TNeighbors>
    [[nodiscard]] PsizeMetric estimateWithNeighbors(const TNeighbors& neighbors, WrapSSIM& wrapAnchor) noexcept;

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
    [[nodiscard]] TLCT_API const TBridge& getBridge() const noexcept { return bridge_; }

    // Non-const methods
    [[nodiscard]] TLCT_API TBridge& getBridge() noexcept { return bridge_; }
    [[nodiscard]] TLCT_API std::expected<void, Error> update(const cv::Mat& src) noexcept;

    // Debug only
    [[nodiscard]] TLCT_API std::expected<void, Error> dumpInfos(const fs::path& dumpTo) const noexcept;
    [[nodiscard]] TLCT_API std::expected<void, Error> loadInfos(const fs::path& loadFrom) noexcept;

private:
    [[nodiscard]] float getPrevPatchsize(int offset) const noexcept { return prevPatchInfos_[offset].getPatchsize(); }
    [[nodiscard]] float getPrevPatchsize(cv::Point index) const noexcept { return getPrevPatchsize(index.y, index.x); }
    [[nodiscard]] float getPrevPatchsize(int row, int col) const noexcept {
        const int offset = row * arrange_.getMIMaxCols() + col;
        return getPrevPatchsize(offset);
    }

    TArrange arrange_;
    TMIBuffers mis_;
    TPInfos prevPatchInfos_;
    TBridge bridge_;
    TPsizeParams params_;
};

}  // namespace tlct::_cvt::ssim

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/impl.cpp"
#endif
