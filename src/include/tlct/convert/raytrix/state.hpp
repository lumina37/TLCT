#pragma once

#include <cmath>
#include <concepts>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "multiview.hpp"
#include "patchsize.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/raytrix.hpp"
#include "tlct/convert/concepts.hpp"
#include "tlct/convert/helper.hpp"

namespace tlct::_cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

class State
{
public:
    static constexpr int CHANNELS = 3;

    // Typename alias
    using TParamConfig = tcfg::ParamConfig;
    using TLayout = TParamConfig::TLayout;
    using TSpecificConfig = TLayout::TSpecificConfig;
    using TMIs = MIs_<TLayout>;

    // Constructor
    State() = delete;
    State(const State& rhs) = delete;
    State& operator=(const State& rhs) = delete;
    TLCT_API inline State(State&& rhs) noexcept = default;
    TLCT_API inline State& operator=(State&& rhs) noexcept = default;
    TLCT_API inline State(TLayout&& layout, TSpecificConfig&& spec_cfg, TMIs&& mis,
                          std::vector<PsizeRecord>&& prev_patchsizes, std::vector<PsizeRecord>&& patchsizes,
                          PsizeParams&& psize_params, MvParams&& mv_params, MvCache&& mv_cache)
        : src_32f_(), layout_(std::move(layout)), spec_cfg_(std::move(spec_cfg)), mis_(std::move(mis)),
          prev_patchsizes_(std::move(prev_patchsizes)), patchsizes_(std::move(patchsizes)),
          psize_params_(std::move(psize_params)), mv_params_(std::move(mv_params)), mv_cache_(std::move(mv_cache)){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline State fromParamCfg(const TParamConfig& param_cfg);

    // Non-const methods
    TLCT_API inline void update(const cv::Mat& src);

    inline void renderInto(cv::Mat& dst, int view_row, int view_col) const
    {
        renderView(src_32f_, dst, layout_, mis_, patchsizes_, mv_params_, mv_cache_, view_row, view_col);
    };

private:
    cv::Mat src_32f_;

    TLayout layout_;
    TSpecificConfig spec_cfg_;
    TMIs mis_;
    std::vector<PsizeRecord> prev_patchsizes_;
    std::vector<PsizeRecord> patchsizes_;

    PsizeParams psize_params_;
    MvParams mv_params_;
    mutable MvCache mv_cache_;
};

static_assert(concepts::CState<State>);

State State::fromParamCfg(const TParamConfig& param_cfg)
{
    const auto& calib_cfg = param_cfg.getCalibCfg();
    auto spec_cfg = param_cfg.getSpecificCfg();
    auto layout = TLayout::fromCalibAndSpecConfig(calib_cfg, spec_cfg).upsample(spec_cfg.getUpsample());

    auto mis = TMIs::fromLayout(layout);

    auto prev_patchsizes = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols(), PsizeRecord{});
    auto patchsizes = std::vector<PsizeRecord>(layout.getMIRows() * layout.getMIMaxCols());
    auto psize_params = PsizeParams::fromConfigs(layout, spec_cfg);

    const int views = param_cfg.getGenericCfg().getViews();
    auto mv_params = MvParams::fromConfigs(layout, spec_cfg, views);
    auto mv_cache = MvCache::fromParams(mv_params);

    return {std::move(layout),     std::move(spec_cfg),     std::move(mis),       std::move(prev_patchsizes),
            std::move(patchsizes), std::move(psize_params), std::move(mv_params), std::move(mv_cache)};
}

void State::update(const cv::Mat& src)
{
    layout_.processInto(src, src_32f_);
    mis_.update(src_32f_);
    src_32f_.convertTo(src_32f_, CV_32FC3);

    std::swap(prev_patchsizes_, patchsizes_);
    estimatePatchsizes(layout_, spec_cfg_, psize_params_, mis_, prev_patchsizes_, patchsizes_);
}

} // namespace tlct::_cvt::raytrix
