#pragma once

#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/config.hpp"
#include "tlct/convert/common/cache.hpp"
#include "tlct/convert/concepts/bridge.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview/ltype_merge/cache.hpp"
#include "tlct/convert/multiview/params.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
#include "tlct/io/yuv.hpp"

namespace tlct::_cvt::lm {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange_>
class MvImpl_ {
public:
    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TCommonCache = CommonCache_<TArrange>;

private:
    using TMvParams = MvParams_<TArrange>;
    using TMvCache = MvCache_<TArrange>;

    MvImpl_(const TArrange& arrange, const TMvParams& params, TMvCache&& cache,
            std::shared_ptr<TCommonCache>&& pCommonCache) noexcept;

public:
    // Constructor
    MvImpl_() = delete;
    MvImpl_(const MvImpl_& rhs) = delete;
    MvImpl_& operator=(const MvImpl_& rhs) = delete;
    MvImpl_(MvImpl_&& rhs) noexcept = default;
    MvImpl_& operator=(MvImpl_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MvImpl_, Error> create(
        const TArrange& arrange, const TCvtConfig& cvtCfg, std::shared_ptr<TCommonCache> pCommonCache) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Size getOutputSize() const noexcept {
        return {params_.outputWidth, params_.outputHeight};
    }

    template <concepts::CPatchMergeBridge TBridge>
    [[nodiscard]] std::expected<void, Error> renderView(const TBridge& bridge, io::YuvPlanarFrame& dst, int viewRow,
                                                        int viewCol) const noexcept;

private:
    template <concepts::CPatchMergeBridge TBridge>
    [[nodiscard]] std::expected<void, Error> genLenTypeWeight(const TBridge& bridge, const cv::Mat& src,
                                                              std::array<cv::Mat, 3>& lenTypeWeights, int viewRow,
                                                              int viewCol) const noexcept;
    template <concepts::CPatchMergeBridge TBridge>
    [[nodiscard]] std::expected<void, Error> renderChan(const TBridge& bridge,
                                                        const std::array<cv::Mat, 3>& lenTypeWeights,
                                                        const cv::Mat& src, cv::Mat& dst, cv::Size dstSize, int viewRow,
                                                        int viewCol) const noexcept;

    TArrange arrange_;
    TMvParams params_;
    std::shared_ptr<TCommonCache> pCommonCache_;
    mutable TMvCache mvCache_;
};

template <cfg::concepts::CArrange TArrange>
template <concepts::CPatchMergeBridge TBridge>
std::expected<void, Error> MvImpl_<TArrange>::renderView(const TBridge& bridge, io::YuvPlanarFrame& dst, int viewRow,
                                                         int viewCol) const noexcept {
    // TODO: handle `std::bad_alloc` in this func
    std::array<std::reference_wrapper<cv::Mat>, TCommonCache::CHANNELS> channels{
        std::ref(dst.getY()), std::ref(dst.getU()), std::ref(dst.getV())};
    const io::YuvPlanarExtent& frameExtent = dst.getExtent();
    std::array<cv::Size, TCommonCache::CHANNELS> channelSizes{
        frameExtent.getYSize(),
        frameExtent.getUSize(),
        frameExtent.getVSize(),
    };

    if (arrange_.getDirection()) {
        for (auto& channelSize : channelSizes) {
            std::swap(channelSize.width, channelSize.height);
        }
    }

    // gen len type weight
    std::array<cv::Mat, 3> lenTypeWeights;
    for (auto& lenTypeWeight : lenTypeWeights) {
        lenTypeWeight.create(params_.getRoiSize(), CV_32FC1);
    }
    auto genWeightRes = genLenTypeWeight(bridge, pCommonCache_->srcs[0], lenTypeWeights, viewRow, viewCol);
    if (!genWeightRes) return std::unexpected{std::move(genWeightRes.error())};

    // render channels
    for (const int chanIdx : rgs::views::iota(0, TCommonCache::CHANNELS)) {
        auto renderChanRes = renderChan(bridge, lenTypeWeights, pCommonCache_->srcs[chanIdx], channels[chanIdx],
                                        channelSizes[chanIdx], viewRow, viewCol);
        if (!renderChanRes) return std::unexpected{std::move(renderChanRes.error())};
    }

    if (arrange_.getDirection()) {
        cv::transpose(dst.getY(), dst.getY());
        cv::transpose(dst.getU(), dst.getU());
        cv::transpose(dst.getV(), dst.getV());
    }

    return {};
}

template <cfg::concepts::CArrange TArrange>
template <concepts::CPatchMergeBridge TBridge>
std::expected<void, Error> MvImpl_<TArrange>::genLenTypeWeight(const TBridge& bridge, const cv::Mat& src,
                                                               std::array<cv::Mat, 3>& lenTypeWeights, int viewRow,
                                                               int viewCol) const noexcept {
    const int viewShiftX = (viewCol - params_.views / 2) * params_.viewInterval;
    const int viewShiftY = (viewRow - params_.views / 2) * params_.viewInterval;

    src.convertTo(mvCache_.f32Chan, CV_32FC1);

    cv::Mat resizedPatch;
    cv::Mat rotatedPatch;
    cv::Mat blendedPatch;
    cv::Mat patchGradsMap;

    const cfg::MITypes miTypes{arrange_.isOutShift()};
    for (int lenType = 0; lenType < 3; lenType++) {
        mvCache_.renderCanvas.setTo(0);
        mvCache_.weightCanvas.setTo(0);

        for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
                const int miType = miTypes.getMIType(row, col);
                if (miType != lenType) {
                    continue;
                }

                // Extract patch
                const cv::Point2f center = arrange_.getMICenter(row, col);
                const float psize = params_.psizeInflate * bridge.getPatchsize(row, col);
                const cv::Point2f patchCenter{center.x + viewShiftX, center.y + viewShiftY};
                const cv::Mat& patch = getRoiImageByCenter(mvCache_.f32Chan, patchCenter, psize);

                // Paste patch
                if (arrange_.isKepler()) {
                    cv::resize(patch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_CUBIC);
                } else {
                    cv::rotate(patch, rotatedPatch, cv::ROTATE_180);
                    cv::resize(rotatedPatch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_CUBIC);
                }

                computeGradsMap(resizedPatch, patchGradsMap);
                cv::multiply(patchGradsMap, mvCache_.gradBlendingWeight4Grads, blendedPatch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int rightShift = ((row % 2) ^ (int)arrange_.isOutShift()) * (params_.patchXShift / 2);
                const cv::Rect roi{col * params_.patchXShift + rightShift, row * params_.patchYShift,
                                   params_.resizedPatchWidth, params_.resizedPatchWidth};

                mvCache_.renderCanvas(roi) += blendedPatch;
                mvCache_.weightCanvas(roi) += mvCache_.gradBlendingWeight4Grads;
            }
        }

        cv::divide(mvCache_.renderCanvas, mvCache_.weightCanvas, mvCache_.renderCanvas);
        cv::Mat croppedGradsMap = mvCache_.renderCanvas(params_.canvasCropRoi);
        croppedGradsMap.copyTo(lenTypeWeights[lenType]);
    }

    return {};
}

template <cfg::concepts::CArrange TArrange>
template <concepts::CPatchMergeBridge TBridge>
std::expected<void, Error> MvImpl_<TArrange>::renderChan(const TBridge& bridge,
                                                         const std::array<cv::Mat, 3>& lenTypeWeights,
                                                         const cv::Mat& src, cv::Mat& dst, cv::Size dstSize,
                                                         int viewRow, int viewCol) const noexcept {
    const int viewShiftX = (viewCol - params_.views / 2) * params_.viewInterval;
    const int viewShiftY = (viewRow - params_.views / 2) * params_.viewInterval;

    cv::Mat renderCanvas(cv::Size{params_.canvasCropRoi[1].size(), params_.canvasCropRoi[0].size()},
                         mvCache_.renderCanvas.type());
    cv::Mat weightCanvas(cv::Size{params_.canvasCropRoi[1].size(), params_.canvasCropRoi[0].size()},
                         mvCache_.weightCanvas.type());
    renderCanvas.setTo(std::numeric_limits<float>::epsilon());
    weightCanvas.setTo(std::numeric_limits<float>::epsilon());

    src.convertTo(mvCache_.f32Chan, CV_32FC1);

    cv::Mat resizedPatch;
    cv::Mat rotatedPatch;
    cv::Mat blendedPatch;

    const cfg::MITypes miTypes{arrange_.isOutShift()};
    for (int lenType = 0; lenType < 3; lenType++) {
        mvCache_.renderCanvas.setTo(0);
        mvCache_.weightCanvas.setTo(0);

        for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
                const int miType = miTypes.getMIType(row, col);
                if (miType != lenType) {
                    continue;
                }

                // Extract patch
                const cv::Point2f center = arrange_.getMICenter(row, col);
                const float psize = params_.psizeInflate * bridge.getPatchsize(row, col);
                const cv::Point2f patchCenter{center.x + viewShiftX, center.y + viewShiftY};
                const cv::Mat& patch = getRoiImageByCenter(mvCache_.f32Chan, patchCenter, psize);

                // Paste patch
                if (arrange_.isKepler()) {
                    cv::resize(patch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_CUBIC);
                } else {
                    cv::rotate(patch, rotatedPatch, cv::ROTATE_180);
                    cv::resize(rotatedPatch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_CUBIC);
                }

                cv::multiply(resizedPatch, mvCache_.gradBlendingWeight, blendedPatch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int rightShift = ((row % 2) ^ (int)arrange_.isOutShift()) * (params_.patchXShift / 2);
                const cv::Rect roi{col * params_.patchXShift + rightShift, row * params_.patchYShift,
                                   params_.resizedPatchWidth, params_.resizedPatchWidth};

                mvCache_.renderCanvas(roi) += blendedPatch;
                mvCache_.weightCanvas(roi) += mvCache_.gradBlendingWeight;
            }
        }

        cv::Mat croppedRenderCanvas = mvCache_.renderCanvas(params_.canvasCropRoi);
        cv::multiply(croppedRenderCanvas, lenTypeWeights[lenType], croppedRenderCanvas);
        renderCanvas += croppedRenderCanvas;

        cv::Mat croppedWeightCanvas = mvCache_.weightCanvas(params_.canvasCropRoi);
        cv::multiply(croppedWeightCanvas, lenTypeWeights[lenType], croppedWeightCanvas);
        weightCanvas += croppedWeightCanvas;
    }

    cv::divide(renderCanvas, weightCanvas, mvCache_.u8NormedImage, 1, CV_8UC1);
    cv::resize(mvCache_.u8NormedImage, dst, dstSize, 0.0, 0.0, cv::INTER_CUBIC);

    return {};
}

}  // namespace tlct::_cvt::lm

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/ltype_merge/impl.cpp"
#endif
