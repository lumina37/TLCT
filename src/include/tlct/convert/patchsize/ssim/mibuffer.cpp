#include <format>
#include <memory>
#include <ranges>
#include <vector>

#include <opencv2/imgproc.hpp>

#include "tlct/config/arrange.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/convert/helper/functional.hpp"
#include "tlct/convert/helper/roi.hpp"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/mibuffer.hpp"
#endif

namespace tlct::_cvt::ssim {

namespace rgs = std::ranges;

template <cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::MIBuffers_(TArrange&& arrange, Params&& params, std::vector<MIBuffer>&& miBuffers,
                                 std::unique_ptr<std::byte[]>&& pBuffer) noexcept
    : arrange_(std::move(arrange)),
      params_(std::move(params)),
      miBuffers_(std::move(miBuffers)),
      pBuffer_(std::move(pBuffer)) {}

template <cfg::concepts::CArrange TArrange>
MIBuffers_<TArrange>::Params::Params(const TArrange& arrange) noexcept {
    idiameter_ = _hp::iround(arrange.getDiameter());
    alignedMatSize_ = _hp::alignUp<SIMD_FETCH_SIZE>(idiameter_ * idiameter_ * sizeof(float));
    alignedMISize_ = 2 * alignedMatSize_;
    miMaxCols_ = arrange.getMIMaxCols();
    miNum_ = miMaxCols_ * arrange.getMIRows();
    bufferSize_ = miNum_ * alignedMISize_;
}

template <cfg::concepts::CArrange TArrange>
auto MIBuffers_<TArrange>::create(const TArrange& arrange) noexcept -> std::expected<MIBuffers_, Error> {
    auto copiedArrange = arrange;
    Params params{arrange};
    try {
        std::vector<MIBuffer> miBuffers(params.miNum_);
        auto pBuffer = std::make_unique_for_overwrite<std::byte[]>(params.bufferSize_ + Params::SIMD_FETCH_SIZE);
        return MIBuffers_{std::move(copiedArrange), std::move(params), std::move(miBuffers), std::move(pBuffer)};
    } catch (const std::bad_alloc&) {
        return std::unexpected{Error{ECate::eSys, ECode::eOutOfMemory}};
    }
}

template <cfg::concepts::CArrange TArrange>
std::expected<void, Error> MIBuffers_<TArrange>::update(const cv::Mat& src) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    if (src.type() != CV_8UC1) [[unlikely]] {
        auto errMsg = std::format("MIBuffers::update expect CV_8UC1, got {}", src.type());
        return std::unexpected{Error{ECate::eTLCT, ECode::eUnexValue, std::move(errMsg)}};
    }

    cv::Mat f32I2;
    const cv::Mat& f32I = src;
    cv::multiply(f32I, f32I, f32I2);

    auto itemIt = miBuffers_.begin();
    uint8_t* rowCursor = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)pBuffer_.get());
    size_t rowStep = params_.miMaxCols_ * params_.alignedMISize_;
    for (const int irow : rgs::views::iota(0, arrange_.getMIRows())) {
        uint8_t* colCursor = rowCursor;
        const int miCols = arrange_.getMICols(irow);
        for (const int icol : rgs::views::iota(0, miCols)) {
            const cv::Point2f& miCenter = arrange_.getMICenter(irow, icol);
            const cv::Rect roi = getRoiByCenter(miCenter, arrange_.getDiameter());

            uint8_t* matCursor = colCursor;

            const cv::Mat& srcI = f32I(roi);
            cv::Mat dstI = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matCursor);
            srcI.copyTo(dstI);
            matCursor += params_.alignedMatSize_;

            const cv::Mat& srcI2 = f32I2(roi);
            cv::Mat dstI2 = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matCursor);
            srcI2.copyTo(dstI2);
            matCursor += params_.alignedMatSize_;

            const float grads = computeGrads(dstI);
            itemIt->grads = grads;

            itemIt->dhash = dhash(dstI);

            *itemIt = {std::move(dstI), std::move(dstI2)};
            itemIt++;
            colCursor += params_.alignedMISize_;
        }

        if (miCols < params_.miMaxCols_) {
            itemIt++;
        }

        rowCursor += rowStep;
    }

    return {};
}

template class MIBuffers_<cfg::CornersArrange>;
template class MIBuffers_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::ssim
