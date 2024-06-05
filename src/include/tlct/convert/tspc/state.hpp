#pragma once

#include <ranges>
#include <string>

#include <opencv2/core.hpp>

#include "helper.hpp"
#include "tlct/common/defines.h"
#include "tlct/config/tspc.hpp"
#include "tlct/convert/concepts/state.hpp"

namespace tlct::cvt::tspc {

namespace tcfg = tlct::cfg;
namespace rgs = std::ranges;

class State
{
public:
    // Typename alias
    using TLayout = tcfg::tspc::Layout;

    // Constructor
    State() = delete;
    TLCT_API inline State(const State& cfg) noexcept = default;
    TLCT_API inline State(State&& cfg) noexcept = default;
    TLCT_API inline State(const TLayout& layout, int views);

    // Initialize from
    [[nodiscard]] TLCT_API static inline State fromLayoutAndViews(const TLayout& layout, int views);

    // Non-const methods
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

    TLCT_API friend inline cv::Mat estimatePatchsizes(const State& state);
    TLCT_API friend inline cv::Mat renderView(const State& state, int view_row, int view_col);

private:
    const TLayout& layout_;
    int views_;
    cv::Mat prev_patchsizes_;
    cv::Mat patchsizes_;
    cv::Mat gray_src_;
    cv::Mat src_32f_;
    int patch_resize_width_; // the extracted patch will be zoomed to this height
    int patch_resize_height_;
    int bound_;
    int p_resize_width_withbound_;
    cv::Mat patch_fadeout_weight_;
    int move_range_;
    int interval_;
    int canvas_width_;
    int canvas_height_;
    int final_width_;
    int final_height_;
    cv::Range canvas_crop_roi_[2];
};

static_assert(concepts::CState<State>);

State::State(const TLayout& layout, int views)
    : layout_(layout), views_(views), prev_patchsizes_(), patchsizes_(), src_32f_()
{
    const int upsample = layout.getUpsample();
    // This indirectly controls the final output size. DO NOT make it too large.
    constexpr double patch_resize_width_factor = 0.285;
    patch_resize_width_ = (int)std::round(patch_resize_width_factor * layout.getDiameter());
    patch_resize_height_ = (int)std::round((double)patch_resize_width_ * std::numbers::sqrt3 / 2.0);
    // Block effect if the bound is too small. Blurring if the bound is too large.
    constexpr double patch_bound_factor = 0.06;
    bound_ = (int)std::round(patch_bound_factor * layout.getDiameter());

    p_resize_width_withbound_ = patch_resize_width_ + 2 * bound_;
    patch_fadeout_weight_ = _hp::rectWithFadeoutBorder({p_resize_width_withbound_, p_resize_width_withbound_}, bound_);

    // Controls the view shift.
    // The distance between the patches from the left and middle viewpoint is `move_range_` pix.
    constexpr double move_range_factor = 0.085;
    move_range_ = (int)std::round(move_range_factor * layout.getDiameter() * 2.0);
    interval_ = views > 1 ? move_range_ / (views - 1) : 0;

    canvas_width_ = (layout.getMIMinCols() - 1) * patch_resize_width_ + 2 * bound_ + patch_resize_width_ / 2;
    canvas_height_ = layout.getMIRows() * patch_resize_height_ + p_resize_width_withbound_ - patch_resize_height_;
    final_width_ = tlct::_hp::align_to_2((int)std::round((double)canvas_width_ / upsample));
    final_height_ = tlct::_hp::align_to_2((int)std::round((double)canvas_height_ / upsample));

    canvas_crop_roi_[0] = cv::Range::all();
    canvas_crop_roi_[1] = cv::Range{p_resize_width_withbound_ / 2, canvas_width_ - p_resize_width_withbound_ / 2};
}

State State::fromLayoutAndViews(const TLayout& layout, int views) { return {layout, views}; }

void State::feed(const cv::Mat& newsrc)
{
    cv::Mat proced_src;
    TLayout::procImg_(layout_, newsrc, proced_src);
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

} // namespace tlct::cvt::tspc