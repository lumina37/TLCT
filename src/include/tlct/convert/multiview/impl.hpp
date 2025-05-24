#pragma once

#include <expected>
#include <ranges>

#include <opencv2/imgproc.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/concepts/psize.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview/cache.hpp"
#include "tlct/convert/multiview/params.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io.hpp"

namespace tlct::_cvt {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange_>
class MvImpl_ {
public:
    // Typename alias
    using TCvtConfig = cfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using MvParams = MvParams_<TArrange>;
    using MvCache = MvCache_<TArrange>;

private:
    MvImpl_(const TArrange& arrange, const MvParams& params, MvCache&& cache) noexcept;

public:
    // Constructor
    MvImpl_() = delete;
    MvImpl_(const MvImpl_& rhs) = delete;
    MvImpl_& operator=(const MvImpl_& rhs) = delete;
    MvImpl_(MvImpl_&& rhs) noexcept = default;
    MvImpl_& operator=(MvImpl_&& rhs) noexcept = default;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<MvImpl_, Error> create(const TArrange& arrange,
                                                                       const TCvtConfig& cvtCfg) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Size getOutputSize() const noexcept {
        return {params_.outputWidth, params_.outputHeight};
    }

    [[nodiscard]] TLCT_API auto& getSrcChans() noexcept { return cache_.srcs; }
    [[nodiscard]] TLCT_API const auto& getSrcChans() const noexcept { return cache_.srcs; }

    [[nodiscard]] TLCT_API auto& getDstChans() noexcept { return cache_.u8OutputImageChannels; }
    [[nodiscard]] TLCT_API const auto& getDstChans() const noexcept { return cache_.u8OutputImageChannels; }

    template <concepts::CPsizeImpl TPsizeImpl>
    [[nodiscard]] TLCT_API std::expected<void, Error> renderView(const TPsizeImpl& psizeImpl, int viewRow,
                                                                 int viewCol) const noexcept;

    // Non-const methods
    [[nodiscard]] TLCT_API std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;

private:
    TArrange arrange_;
    MvParams params_;
    mutable MvCache cache_;
};

template <cfg::concepts::CArrange TArrange>
template <concepts::CPsizeImpl TPsizeImpl>
std::expected<void, Error> MvImpl_<TArrange>::renderView(const TPsizeImpl& psizeImpl, int viewRow,
                                                         int viewCol) const noexcept {
    // TODO: handle `std::bad_alloc` in this func
    const int viewShiftX = (viewCol - params_.views / 2) * params_.viewInterval;
    const int viewShiftY = (viewRow - params_.views / 2) * params_.viewInterval;

    cv::Mat resizedPatch;
    [[maybe_unused]] cv::Mat rotatedPatch;
    cv::Mat weightedPatch;

    for (const int chanIdx : rgs::views::iota(0, (int)cache_.srcs.size())) {
        cache_.renderCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache_.weightCanvas.setTo(std::numeric_limits<float>::epsilon());
        cache_.srcs[chanIdx].convertTo(cache_.f32Chan, CV_32FC1);

        for (const int row : rgs::views::iota(0, arrange_.getMIRows())) {
            for (const int col : rgs::views::iota(0, arrange_.getMICols(row))) {
                // Extract patch
                const cv::Point2f center = arrange_.getMICenter(row, col);
                const float psize = params_.psizeInflate * psizeImpl.getPatchsize(row, col);
                const cv::Point2f patchCenter{center.x + viewShiftX, center.y + viewShiftY};
                const cv::Mat& patch = getRoiImageByCenter(cache_.f32Chan, patchCenter, psize);

                // Paste patch
                if (arrange_.isKepler()) {
                    cv::rotate(patch, rotatedPatch, cv::ROTATE_180);
                    cv::resize(rotatedPatch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                } else {
                    cv::resize(patch, resizedPatch, {params_.resizedPatchWidth, params_.resizedPatchWidth}, 0, 0,
                               cv::INTER_LINEAR_EXACT);
                }

                cv::multiply(resizedPatch, cache_.gradBlendingWeight, weightedPatch);

                // if the second bar is not out shift, then we need to shift the 1 col
                // else if the second bar is out shift, then we need to shift the 0 col
                const int rightShift = ((row % 2) ^ (int)arrange_.isOutShift()) * (params_.patchXShift / 2);
                const cv::Rect roi{col * params_.patchXShift + rightShift, row * params_.patchYShift,
                                   params_.resizedPatchWidth, params_.resizedPatchWidth};

                if (arrange_.isMultiFocus()) {
                    const float weight = psizeImpl.getWeight(row, col);
                    cache_.renderCanvas(roi) += weightedPatch * weight;
                    cache_.weightCanvas(roi) += cache_.gradBlendingWeight * weight;
                } else {
                    cache_.renderCanvas(roi) += weightedPatch;
                    cache_.weightCanvas(roi) += cache_.gradBlendingWeight;
                }
            }
        }

        cv::Mat croppedRenderedImage = cache_.renderCanvas(params_.canvasCropRoi);
        cv::Mat croppedWeightMatrix = cache_.weightCanvas(params_.canvasCropRoi);

        cv::divide(croppedRenderedImage, croppedWeightMatrix, cache_.u8NormedImage, 1, CV_8UC1);
        cv::resize(cache_.u8NormedImage, cache_.u8OutputImageChannels[chanIdx],
                   {params_.outputWidth, params_.outputHeight}, 0.0, 0.0, cv::INTER_LINEAR_EXACT);
    }

    return {};
}

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/multiview/impl.cpp"
#endif
