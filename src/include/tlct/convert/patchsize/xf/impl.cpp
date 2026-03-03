#include <format>
#include <limits>
#include <queue>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/config.hpp"
#include "tlct/convert/concepts/neighbors.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/convert/patchsize/ssim/functional.hpp"
#include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#include "tlct/convert/patchsize/ssim/params.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/xf/impl.hpp"
#endif

namespace tlct::_cvt::xf {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
PsizeImpl_<TArrange>::PsizeImpl_(const TArrange& arrange, TMIBuffers&& mis, TMIBuffers&& prevMis,
                                 TPInfos&& prevPatchInfos, const TPsizeParams& params) noexcept
    : arrange_(arrange),
      mis_(std::move(mis)),
      prevMis_(std::move(prevMis)),
      prevPatchInfos_(std::move(prevPatchInfos)),
      params_(params) {}

template <cfg::concepts::CArrange TArrange>
PsizeMetric PsizeImpl_<TArrange>::estimateWithNeighbors(const NearNeighbors_<TArrange>& neighbors,
                                                        ssim::WrapSSIM& wrapAnchor) const noexcept {
    using NearNeighbors = NearNeighbors_<TArrange>;

    const cv::Point2f miCenter{arrange_.getRadius(), arrange_.getRadius()};
    const int maxShift = (int)(params_.patternShift * 2);

    typename NearNeighbors::Direction direction = NearNeighbors::Direction::RIGHT;
    if (!neighbors.hasNeighbor(direction)) [[unlikely]] {
        direction = NearNeighbors::Direction::LEFT;
    }

    const cv::Point2f anchorShift =
        _hp::sgn(arrange_.isKepler()) * NearNeighbors::getUnitShift(direction) * params_.patternShift;
    const cv::Rect anchorRoi = getRoiByCenter(miCenter + anchorShift, params_.patternSize);
    wrapAnchor.updateRoi(anchorRoi);

    const ssim::MIBuffer& neibMI = mis_.getMI(neighbors.getNeighborIdx(direction));
    ssim::WrapSSIM wrapNeib{neibMI};

    const cv::Point2f matchStep = -_hp::sgn(arrange_.isKepler()) * NearNeighbors::getUnitShift(direction);
    cv::Point2f cmpShift = anchorShift + matchStep * params_.minPsize;

    int bestPsize = 0;
    float maxSsim = std::numeric_limits<float>::lowest();
    for (const int psize : rgs::views::iota(params_.minPsize, maxShift)) {
        cmpShift += matchStep;

        const cv::Rect cmpRoi = getRoiByCenter(miCenter + cmpShift, params_.patternSize);
        wrapNeib.updateRoi(cmpRoi);

        const float ssim = wrapAnchor.compare(wrapNeib);
        if (ssim > maxSsim) {
            maxSsim = ssim;
            bestPsize = psize;
        }
    }

    const float metric = maxSsim * maxSsim;

    const float clipedSumPsize = _hp::clip((float)bestPsize, (float)params_.minPsize, (float)maxShift);
    const float psize = clipedSumPsize / NearNeighbors::INFLATE;

    return {psize, metric};
}

template <cfg::concepts::CArrange TArrange>
float PsizeImpl_<TArrange>::estimatePatchsize(TBridge& bridge, cv::Point index) const noexcept {
    using PsizeParams = ssim::PsizeParams_<TArrange>;

    const int offset = index.y * arrange_.getMIMaxCols() + index.x;
    const ssim::MIBuffer& anchorMI = mis_.getMI(offset);
    const float prevPsize = prevPatchInfos_[offset].getPatchsize();

    ssim::WrapSSIM wrapAnchor{anchorMI};
    if (prevPsize != PsizeParams::INVALID_PSIZE) [[likely]] {
        const cv::Point2f miCenter{arrange_.getRadius(), arrange_.getRadius()};
        const cv::Rect roi = getRoiByCenter(miCenter, arrange_.getDiameter() / std::numbers::sqrt2_v<float>);
        wrapAnchor.updateRoi(roi);

        const ssim::MIBuffer& prevMI = prevMis_.getMI(offset);
        ssim::WrapSSIM wrapPrev{prevMI};
        wrapPrev.updateRoi(roi);

        const float ssim = wrapAnchor.compare(wrapPrev);
        if (ssim >= params_.psizeShortcutThreshold) {
            bridge.getInfo(offset).setInherited(true);
            return prevPsize;
        }
    }

    const NearNeighbors& nearNeighbors = NearNeighbors::fromArrangeAndIndex(arrange_, index);
    const PsizeMetric& nearPsizeMetric = estimateWithNeighbors(nearNeighbors, wrapAnchor);
    const float bestPsize = nearPsizeMetric.psize;

    return bestPsize;
}

template <cfg::concepts::CArrange TArrange>
auto PsizeImpl_<TArrange>::create(const TArrange& arrange, const TCvtConfig& cvtCfg) noexcept
    -> std::expected<PsizeImpl_, Error> {
    auto misRes = TMIBuffers::create(arrange);
    if (!misRes) return std::unexpected{std::move(misRes.error())};
    auto& mis = misRes.value();

    auto prevMisRes = TMIBuffers::create(arrange);
    if (!prevMisRes) return std::unexpected{std::move(prevMisRes.error())};
    auto& prevMis = prevMisRes.value();

    std::vector<TPInfo> prevPatchInfos(arrange.getMIRows() * arrange.getMIMaxCols());

    auto paramsRes = TPsizeParams::create(arrange, cvtCfg);
    if (!paramsRes) return std::unexpected{std::move(paramsRes.error())};
    auto& params = paramsRes.value();

    return PsizeImpl_{arrange, std::move(mis), std::move(prevMis), std::move(prevPatchInfos), params};
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> PsizeImpl_<TArrange>::updateBridge(const cv::Mat& src, TBridge& bridge) noexcept {
    std::swap(mis_, prevMis_);
    bridge.swapInfos(prevPatchInfos_);

    auto updateRes = mis_.update(src);
    if (!updateRes) return std::unexpected{std::move(updateRes.error())};

#pragma omp parallel for
    for (int idx = 0; idx < (int)prevPatchInfos_.size(); idx++) {
        const int row = idx / arrange_.getMIMaxCols();
        const int col = idx % arrange_.getMIMaxCols();
        if (col >= arrange_.getMICols(row)) {
            continue;
        }
        const cv::Point index{col, row};
        const float psize = estimatePatchsize(bridge, index);
        bridge.getInfo(idx).setPatchsize(psize);
    }

    return {};
}

template class PsizeImpl_<cfg::CornersArrange>;
template class PsizeImpl_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::xf
