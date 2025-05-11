#pragma once

#include <ranges>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/common.hpp"
#include "tlct/config/concepts/arrange.hpp"
#include "tlct/convert/helper.hpp"
#include "tlct/convert/multiview.hpp"
#include "tlct/convert/patchsize.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/io.hpp"

namespace tlct {

namespace _cvt {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class Manager_ {
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TError = Error;
    using TCvtConfig = tcfg::CliConfig::Convert;
    using TArrange = TArrange_;
    using TMIBuffers = MIBuffers_<TArrange>;
    using PsizeParams = PsizeParams_<TArrange>;
    using MvParams = MvParams_<TArrange>;
    using MvCache = MvCache_<TArrange>;

    // Constructor
    Manager_() = delete;
    Manager_(const Manager_& rhs) = delete;
    Manager_& operator=(const Manager_& rhs) = delete;
    Manager_(Manager_&& rhs) noexcept = default;
    Manager_& operator=(Manager_&& rhs) noexcept = default;
    Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg);

    // Initialize from
    [[nodiscard]] static std::expected<Manager_, TError> create(const TArrange& arrange, const TCvtConfig& cvtCfg);

    // Const methods
    [[nodiscard]] cv::Size getOutputSize() const noexcept { return {mvParams_.outputWidth, mvParams_.outputHeight}; };

    // Non-const methods
    [[nodiscard]] std::expected<void, Error> update(const io::YuvPlanarFrame& src) noexcept;

    [[nodiscard]] std::expected<void, Error> renderInto(io::YuvPlanarFrame& dst, int viewRow,
                                                        int viewCol) const noexcept {
        {
            auto res = renderView<TArrange>(mvCache_.srcs, mvCache_.u8OutputImageChannels, arrange_, mvParams_,
                                            patchsizes_, mvCache_, viewRow, viewCol);
            if (!res) return std::unexpected{std::move(res.error())};
        }

        if (arrange_.getDirection()) {
            for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
                cv::transpose(mvCache_.u8OutputImageChannels[i], mvCache_.u8OutputImageChannels[i]);
            }
        }

        mvCache_.u8OutputImageChannels[0].copyTo(dst.getY());
        cv::resize(mvCache_.u8OutputImageChannels[1], dst.getU(),
                   {dst.getExtent().getUWidth(), dst.getExtent().getUHeight()}, 0.0, 0.0, cv::INTER_LINEAR_EXACT);
        cv::resize(mvCache_.u8OutputImageChannels[2], dst.getV(),
                   {dst.getExtent().getVWidth(), dst.getExtent().getVHeight()}, 0.0, 0.0, cv::INTER_LINEAR_EXACT);

        return {};
    }

private:
    TArrange arrange_;
    TCvtConfig cvtCfg_;
    TMIBuffers mis_;
    cv::Mat prevPatchsizes_;  // CV_32FC1
    cv::Mat patchsizes_;      // CV_32FC1
    PsizeParams psizeParams_;
    MvParams mvParams_;
    mutable MvCache mvCache_;
};

template <tcfg::concepts::CArrange TArrange>
Manager_<TArrange>::Manager_(const TArrange& arrange, const TCvtConfig& cvtCfg) : arrange_(arrange), cvtCfg_(cvtCfg) {
    mis_ = TMIBuffers::create(arrange).value();

    prevPatchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);
    patchsizes_ = cv::Mat::zeros(arrange.getMIRows(), arrange.getMIMaxCols(), CV_32FC1);

    psizeParams_ = PsizeParams::create(arrange, cvtCfg).value();
    mvParams_ = MvParams::create(arrange, cvtCfg).value();
    mvCache_ = MvCache::create(mvParams_).value();
}

template <tcfg::concepts::CArrange TArrange>
std::expected<Manager_<TArrange>, typename Manager_<TArrange>::TError> Manager_<TArrange>::create(
    const TArrange& arrange, const TCvtConfig& cvtCfg) {
    return Manager_{arrange, cvtCfg};
}

template <tcfg::concepts::CArrange TArrange>
std::expected<void, Error> Manager_<TArrange>::update(const io::YuvPlanarFrame& src) noexcept {
    // TODO: handle `std::bad_alloc` in this func
    src.getY().copyTo(mvCache_.rawSrcs[0]);
    src.getU().copyTo(mvCache_.rawSrcs[1]);
    src.getV().copyTo(mvCache_.rawSrcs[2]);

    if (arrange_.getDirection()) {
        for (const int i : rgs::views::iota(0, MvCache::CHANNELS)) {
            cv::transpose(mvCache_.rawSrcs[i], mvCache_.rawSrcs[i]);
        }
    }

    const int upsample = arrange_.getUpsample();
    if (upsample != 1) [[likely]] {
        cv::resize(mvCache_.rawSrcs[0], mvCache_.srcs[0], {}, upsample, upsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[0] = mvCache_.rawSrcs[0];
    }

    if (src.getExtent().getUShift() != 0) {
        const int uUpsample = upsample << src.getExtent().getUShift();
        cv::resize(mvCache_.rawSrcs[1], mvCache_.srcs[1], {}, uUpsample, uUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[1] = mvCache_.rawSrcs[1];
    }

    if (src.getExtent().getVShift() != 0) {
        const int vUpsample = upsample << src.getExtent().getVShift();
        cv::resize(mvCache_.rawSrcs[2], mvCache_.srcs[2], {}, vUpsample, vUpsample, cv::INTER_LINEAR_EXACT);
    } else {
        mvCache_.srcs[2] = mvCache_.rawSrcs[2];
    }

    {
        auto res = mis_.update(mvCache_.srcs[0]);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    std::swap(prevPatchsizes_, patchsizes_);

    {
        auto res = estimatePatchsizes<TArrange>(arrange_, cvtCfg_, psizeParams_, mis_, prevPatchsizes_,
                                                                patchsizes_);
        if (!res) return std::unexpected{std::move(res.error())};
    }

    if (arrange_.isMultiFocus()) {
        adjustWgtsAndPsizesForMultiFocus(arrange_, mis_, patchsizes_, mvCache_);
    }

    return {};
}

}  // namespace _cvt

namespace cvt {

using _cvt::Manager_;

}  // namespace cvt

}  // namespace tlct
