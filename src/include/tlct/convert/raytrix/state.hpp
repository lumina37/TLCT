#pragma once

#include <cmath>
#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"
#include "tlct/config/raytrix.hpp"
#include "tlct/convert/concepts/state.hpp"
#include "tlct/convert/helper/inspect.hpp"
#include "tlct/convert/helper/roi.hpp"

namespace tlct::cvt::raytrix {

namespace rgs = std::ranges;
namespace tcfg = tlct::cfg::raytrix;

class State
{
public:
    // Typename alias
    using TParamConfig = tcfg::ParamConfig;
    using TSpecificConfig = TParamConfig::TSpecificConfig;
    using TLayout = tcfg::Layout;

    // Constructor
    State() = delete;
    State(const State& cfg) = delete;
    TLCT_API inline State(State&& cfg) noexcept = default;
    TLCT_API inline State(const TLayout layout, const TSpecificConfig spec_cfg, int views);

    // Initialize from
    [[nodiscard]] TLCT_API static inline State fromParamCfg(const TParamConfig& param_cfg);

    // Non-const methods
    inline void setInspector(_hp::Inspector_<TLayout>&& inspector) noexcept { inspector_ = std::move(inspector); };
    TLCT_API inline void feed(const cv::Mat& newsrc);

    // Iterator
    class iterator
    {
    public:
        // Typename alias
        using value_type = cv::Mat;
        using iterator_category = std::forward_iterator_tag;

        // Constructor
        TLCT_API inline iterator(const State& state, int views, int view_row, int view_col)
            : state_(state), views_(views), view_row_(view_row), view_col_(view_col){};

        // Initialize from
        [[nodiscard]] TLCT_API static inline iterator fromStateAndView(const State& state, int views, int view_row,
                                                                       int view_col) noexcept;

        // Const methods
        [[nodiscard]] TLCT_API inline value_type operator*() const { return renderView(state_, view_row_, view_col_); };
        [[nodiscard]] TLCT_API inline bool operator==(const iterator& rhs) const noexcept
        {
            return view_col_ == rhs.view_col_ && view_row_ == rhs.view_row_;
        }

        // Non-const methods
        TLCT_API inline iterator& operator++() noexcept
        {
            view_col_++;
            if (view_col_ == views_) {
                view_col_ = 0;
                view_row_++;
            }
            return *this;
        };

    private:
        const State& state_;
        int views_;
        int view_row_;
        int view_col_;
    };
    friend class iterator;

    [[nodiscard]] TLCT_API iterator begin() const noexcept { return iterator::fromStateAndView(*this, views_, 0, 0); }
    [[nodiscard]] TLCT_API iterator end() const noexcept
    {
        return iterator::fromStateAndView(*this, views_, views_, 0);
    }

    TLCT_API friend inline cv::Mat estimatePatchsizes(State& state);
    TLCT_API friend inline cv::Mat renderView(const State& state, int view_row, int view_col);

private:
    const TLayout layout_;
    const TSpecificConfig spec_cfg_;
    int views_;
    cv::Mat prev_patchsizes_;
    cv::Mat patchsizes_;
    cv::Mat gray_src_;
    cv::Mat src_32f_;
    int patch_xshift_; // the extracted patch will be zoomed to this height
    int patch_yshift_;
    double bound_;
    int p_resize_withbound_;
    cv::Mat patch_fadeout_weight_;
    int move_range_;
    int interval_;
    int canvas_width_;
    int canvas_height_;
    int final_width_;
    int final_height_;
    cv::Range canvas_crop_roi_[2];
    _hp::Inspector_<TLayout> inspector_;
};

static_assert(concepts::CState<State>);

State::State(const TLayout layout, const TSpecificConfig spec_cfg, int views)
    : layout_(layout), spec_cfg_(spec_cfg), views_(views), prev_patchsizes_(), patchsizes_(), src_32f_(), inspector_()
{
    const int upsample = layout.getUpsample();
    patch_xshift_ = (int)std::round(0.5 * layout.getDiameter());
    patch_yshift_ = (int)std::round((double)patch_xshift_ * std::numbers::sqrt3 / 2.0);
    // Block effect if the bound is too small. Blurring if the bound is too large.
    bound_ = spec_cfg_.getGradientBlendingWidth() * patch_xshift_;

    const double p_resize_withbound_d = patch_xshift_ + 2 * bound_;
    p_resize_withbound_ = (int)std::round(p_resize_withbound_d);
    patch_fadeout_weight_ = _hp::circleWithFadeoutBorder(p_resize_withbound_, (int)std::round(bound_));

    move_range_ = (int)std::round((1.0 + spec_cfg_.getGradientBlendingWidth()) * spec_cfg_.getKernelSize() / 2.0 *
                                  layout.getDiameter());
    interval_ = views > 1 ? move_range_ / (views - 1) : 0;

    canvas_width_ = (int)std::round(layout.getMIMaxCols() * patch_xshift_ + p_resize_withbound_d);
    canvas_height_ = (int)std::round(layout.getMIRows() * patch_yshift_ + p_resize_withbound_d);

    const cv::Range col_range{(int)std::ceil(patch_xshift_ * 1.5),
                              (int)(canvas_width_ - p_resize_withbound_d - patch_xshift_ / 2.0)};
    const cv::Range row_range{(int)std::ceil(patch_xshift_ * 1.5),
                              (int)(canvas_height_ - p_resize_withbound_d - patch_xshift_ / 2.0)};
    canvas_crop_roi_[0] = row_range;
    canvas_crop_roi_[1] = col_range;

    final_width_ = tlct::_hp::align_to_2((int)std::round((double)col_range.size() / upsample));
    final_height_ = tlct::_hp::align_to_2((int)std::round((double)row_range.size() / upsample));
}

State State::fromParamCfg(const TParamConfig& param_cfg)
{
    const auto& spec_cfg = param_cfg.getSpecificCfg();
    const auto layout = TLayout::fromCfgAndImgsize(param_cfg.getCalibCfg(), param_cfg.getSpecificCfg().getImgSize())
                            .upsample(spec_cfg.getUpsample());
    const int views = param_cfg.getGenericCfg().getViews();
    return {layout, spec_cfg, views};
}

void State::feed(const cv::Mat& newsrc)
{
    cv::Mat proced_src;
    layout_.procImg_(newsrc, proced_src);
    cv::cvtColor(proced_src, gray_src_, cv::COLOR_BGR2GRAY);
    proced_src.convertTo(src_32f_, CV_32FC3);

    prev_patchsizes_ = std::move(patchsizes_);
    patchsizes_ = estimatePatchsizes(*this);
    if (prev_patchsizes_.empty()) {
        prev_patchsizes_ = patchsizes_;
    }
}

State::iterator State::iterator::fromStateAndView(const State& state, int views, int view_row, int view_col) noexcept
{
    return {state, views, view_row, view_col};
}

} // namespace tlct::cvt::raytrix