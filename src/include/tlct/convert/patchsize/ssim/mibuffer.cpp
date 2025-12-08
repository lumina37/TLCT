#include <format>
#include <memory>
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

    cv::Mat f32I;
    cv::Mat f32I2;
    src.convertTo(f32I, CV_32FC1);
    cv::multiply(f32I, f32I, f32I2);

    uint8_t* bufBase = (uint8_t*)_hp::alignUp<Params::SIMD_FETCH_SIZE>((size_t)pBuffer_.get());
#pragma omp parallel for
    for (int idx = 0; idx < params_.miNum_; idx++) {
        const int rowMIIdx = idx / params_.miMaxCols_;
        const int colMIIdx = idx % params_.miMaxCols_;
        if (colMIIdx >= arrange_.getMICols(rowMIIdx)) {
            continue;
        }

        auto miBufIterator = miBuffers_.begin() + idx;

        const cv::Point2f& miCenter = arrange_.getMICenter(rowMIIdx, colMIIdx);
        const cv::Rect miRoi = getRoiByCenter(miCenter, arrange_.getDiameter());

        uint8_t* matBufCursor = bufBase + idx * params_.alignedMISize_;

        const cv::Mat& srcI = f32I(miRoi);
        cv::Mat dstI = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matBufCursor);
        srcI.copyTo(dstI);
        matBufCursor += params_.alignedMatSize_;

        const cv::Mat& srcI2 = f32I2(miRoi);
        cv::Mat dstI2 = cv::Mat(params_.idiameter_, params_.idiameter_, CV_32FC1, matBufCursor);
        srcI2.copyTo(dstI2);

        miBufIterator->grads = computeGrads(dstI);
        miBufIterator->I = std::move(dstI);
        miBufIterator->I_2 = std::move(dstI2);
    }

    return {};
}

template class MIBuffers_<cfg::CornersArrange>;
template class MIBuffers_<cfg::OffsetArrange>;

}  // namespace tlct::_cvt::ssim
