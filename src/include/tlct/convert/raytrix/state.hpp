#pragma once

#include <cmath>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "neighbors.hpp"
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
    // Typename alias
    using TParamConfig = tcfg::ParamConfig;
    using TLayout = TParamConfig::TLayout;
    using TSpecificConfig = TLayout::TSpecificConfig;
    using TMIs = MIs<TLayout>;

    static constexpr int INVALID_PSIZE = 0;
    static constexpr int CHANNELS = 3;

    // Constructor
    State() = delete;
    State(const State& rhs) = delete;
    State& operator=(const State& rhs) = delete;
    TLCT_API inline State(State&& rhs) noexcept = default;
    TLCT_API inline State& operator=(State&& rhs) noexcept = default;
    TLCT_API inline State(const TLayout& layout, const TSpecificConfig& spec_cfg, int views);

    // Initialize from
    [[nodiscard]] TLCT_API static inline State fromParamCfg(const TParamConfig& param_cfg);

    // Const methods
    [[nodiscard]] TLCT_API inline const cv::Mat& getPatchsizes() { return patchsizes_; };

    // Non-const methods
#ifdef TLCT_ENABLE_INSPECT
    inline void setInspector(Inspector&& inspector) noexcept { inspector_ = std::move(inspector); };
#endif
    TLCT_API inline void update(const cv::Mat& src);

    inline void estimatePatchsizes();
    inline void renderInto(cv::Mat& dst, int view_row, int view_col) const;
    [[nodiscard]] inline double _calcMetricWithPsize(const Neighbors& neighbors, int psize) const;
    [[nodiscard]] inline int _estimatePatchsizeOverFullMatch(const Neighbors& neighbors);
    [[nodiscard]] inline int _estimatePatchsize(cv::Point index);

private:
#ifdef TLCT_ENABLE_INSPECT
    Inspector inspector_;
#endif
    TLayout layout_;
    TSpecificConfig spec_cfg_;
    TMIs mis_;
    cv::Mat src_32f_;
    cv::Mat prev_patchsizes_;
    cv::Mat patchsizes_;

    // vvv cache vvv
    mutable cv::Mat render_canvas_;
    mutable cv::Mat weight_canvas_;
    mutable cv::Mat cropped_rendered_image_channels_[CHANNELS];
    mutable cv::Mat normed_image_u8_;
    mutable cv::Mat resized_normed_image_u8_;
    // ^^^ cache ^^^

    cv::Mat grad_blending_weight_;
    double grad_blending_bound_;
    double pattern_size_;
    double pattern_shift_;
    cv::Range canvas_crop_roi_[2];
    int views_;
    int patch_xshift_; // the extracted patch will be zoomed to this height
    int patch_yshift_;
    int resized_patch_width_;
    int min_psize_;
    int view_interval_;
    int canvas_width_;
    int canvas_height_;
    int output_width_;
    int output_height_;
};

static_assert(concepts::CState<State>);

State::State(const TLayout& layout, const TSpecificConfig& spec_cfg, int views)
    : layout_(layout), spec_cfg_(spec_cfg), views_(views)
{
    mis_ = TMIs::fromLayout(layout);

    const int upsample = layout.getUpsample();
    const double patch_xshift_d = 0.35 * layout.getDiameter();
    patch_xshift_ = (int)std::ceil(patch_xshift_d);
    patch_yshift_ = (int)std::ceil(patch_xshift_d * std::numbers::sqrt3 / 2.0);

    grad_blending_bound_ = spec_cfg_.getGradientBlendingWidth() * patch_xshift_ * TSpecificConfig::PSIZE_INFLATE;
    const double p_resize_d = patch_xshift_d * TSpecificConfig::PSIZE_INFLATE;
    resized_patch_width_ = (int)std::round(p_resize_d);
    grad_blending_weight_ = circleWithFadeoutBorder(resized_patch_width_, (int)std::round(grad_blending_bound_ / 2));

    pattern_size_ = layout.getDiameter() * spec_cfg.getPatternSize();
    const double radius = layout.getDiameter() / 2.0;
    const double half_pattern_size = pattern_size_ / 2.0;
    const double max_pattern_shift =
        std::sqrt((radius - half_pattern_size) * (radius + half_pattern_size)) - half_pattern_size;
    const double candidate_pattern_shift = radius * spec_cfg.getMaxPatchSize();
    pattern_shift_ = std::min(max_pattern_shift, candidate_pattern_shift);

    min_psize_ = (int)std::round(0.3 * pattern_size_);
    prev_patchsizes_ = cv::Mat(layout_.getMIRows(), layout_.getMIMaxCols(), CV_32SC1);
    patchsizes_ = cv::Mat::zeros(prev_patchsizes_.size(), CV_32SC1);

    const int move_range =
        _hp::iround(layout.getDiameter() * (1.0 - spec_cfg.getMaxPatchSize() * TSpecificConfig::PSIZE_INFLATE));
    view_interval_ = views > 1 ? move_range / (views - 1) : 0;

    canvas_width_ = (int)std::round(layout.getMIMaxCols() * patch_xshift_ + resized_patch_width_);
    canvas_height_ = (int)std::round(layout.getMIRows() * patch_yshift_ + resized_patch_width_);
    render_canvas_ = cv::Mat(canvas_height_, canvas_width_, CV_32FC3);
    weight_canvas_ = cv::Mat(canvas_height_, canvas_width_, CV_32FC1);

    const cv::Range col_range{(int)std::ceil(patch_xshift_ * 1.5),
                              (int)(canvas_width_ - resized_patch_width_ - patch_xshift_ / 2.0)};
    const cv::Range row_range{(int)std::ceil(patch_xshift_ * 1.5),
                              (int)(canvas_height_ - resized_patch_width_ - patch_xshift_ / 2.0)};
    canvas_crop_roi_[0] = row_range;
    canvas_crop_roi_[1] = col_range;

    output_width_ = _hp::round_to<2>(_hp::iround((double)col_range.size() / upsample));
    output_height_ = _hp::round_to<2>(_hp::iround((double)row_range.size() / upsample));
}

State State::fromParamCfg(const TParamConfig& param_cfg)
{
    const auto& calib_cfg = param_cfg.getCalibCfg();
    const auto& spec_cfg = param_cfg.getSpecificCfg();
    const auto layout = TLayout::fromCalibAndSpecConfig(calib_cfg, spec_cfg).upsample(spec_cfg.getUpsample());
    const int views = param_cfg.getGenericCfg().getViews();
    return {layout, spec_cfg, views};
}

void State::update(const cv::Mat& src)
{
    layout_.processInto(src, src_32f_);
    mis_.update(src_32f_);
    src_32f_.convertTo(src_32f_, CV_32FC3);

    std::swap(prev_patchsizes_, patchsizes_);
    estimatePatchsizes();
}

} // namespace tlct::_cvt::raytrix
