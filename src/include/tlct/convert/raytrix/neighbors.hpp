#pragma once

#include <array>

#include "direction.hpp"
#include "tlct/config/raytrix/layout.hpp"

namespace tlct::cvt::raytrix::_hp {

namespace tcfg = tlct::cfg::raytrix;
namespace tcvthp = tlct::cvt::_hp;

template <typename TLayout_>
    requires tlct::cfg::concepts::CLayout<TLayout_>
class Neighbors_
{
public:
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr double DEFAULT_COORD = -1.0;

    // Typename alias
    using TLayout = TLayout_;
    using TIndices = std::array<cv::Point, DIRECTION_NUM>;
    using TPoints = std::array<cv::Point2d, DIRECTION_NUM>;
    using TUnitShifts = std::array<std::array<double, 2>, DIRECTION_NUM>;

    static constexpr double SCALAR_UNIT_SHIFT = 1.0;
    static constexpr double X_UNIT_STEP = std::numbers::sqrt3 / 2.0 * SCALAR_UNIT_SHIFT;
    static constexpr double Y_UNIT_STEP = 0.5 * SCALAR_UNIT_SHIFT;

    static constexpr TUnitShifts UNIT_SHIFTS{{{-X_UNIT_STEP, -Y_UNIT_STEP},
                                              {0.0, -SCALAR_UNIT_SHIFT},
                                              {X_UNIT_STEP, -Y_UNIT_STEP},
                                              {X_UNIT_STEP, Y_UNIT_STEP},
                                              {0.0, SCALAR_UNIT_SHIFT},
                                              {-X_UNIT_STEP, Y_UNIT_STEP}}};

    // Constructor
    Neighbors_() = delete;
    TLCT_API inline Neighbors_& operator=(const Neighbors_& rhs) noexcept = default;
    TLCT_API inline Neighbors_(const Neighbors_& rhs) noexcept = default;
    TLCT_API inline Neighbors_& operator=(Neighbors_&& rhs) noexcept = default;
    TLCT_API inline Neighbors_(Neighbors_&& rhs) noexcept = default;
    TLCT_API inline Neighbors_(const cv::Point self_idx, const TIndices indices, cv::Point2d self_pt,
                               const TPoints points) noexcept
        : self_idx_(self_idx), indices_(indices), self_pt_(self_pt), points_(points){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline Neighbors_ fromLayoutAndIndex(const TLayout& layout,
                                                                       const cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Point getSelfIdx() const noexcept { return self_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point2d getSelfPt() const noexcept { return self_pt_; };

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API inline bool hasNeighbor(const Direction direction) const noexcept
    {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    };
    [[nodiscard]] TLCT_API inline cv::Point getNeighborIdx(const Direction direction) const noexcept
    {
        return indices_[(int)direction];
    };
    [[nodiscard]] TLCT_API inline cv::Point getNeighborPt(const Direction direction) const noexcept
    {
        return points_[(int)direction];
    };
    [[nodiscard]] TLCT_API inline cv::Point2d getUnitShift(const Direction direction) const noexcept
    {
        const auto unit_shift = UNIT_SHIFTS[(int)direction];
        return {unit_shift[0], unit_shift[1]};
    };

private:
    cv::Point self_idx_;
    TIndices indices_;
    cv::Point2d self_pt_;
    TPoints points_;
};

template <typename TLayout>
    requires tlct::cfg::concepts::CLayout<TLayout>
Neighbors_<TLayout> Neighbors_<TLayout>::fromLayoutAndIndex(const TLayout& layout, const cv::Point index) noexcept
{
    cv::Point up_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point down_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point upleft_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point upright_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point downleft_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};
    cv::Point downright_idx{Neighbors_::DEFAULT_INDEX, Neighbors_::DEFAULT_INDEX};

    cv::Point2d self_pt = layout.getMICenter(index);
    cv::Point2d up_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d down_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d upleft_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d upright_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d downleft_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};
    cv::Point2d downright_pt{Neighbors_::DEFAULT_COORD, Neighbors_::DEFAULT_COORD};

    if (index.y > 1) {
        up_idx = {index.x, index.y - 2};
        up_pt = layout.getMICenter(up_idx);
    }
    if (index.y < (layout.getMIRows() - 2)) {
        down_idx = {index.x, index.y + 2};
        down_pt = layout.getMICenter(down_idx);
    }

    const int is_left_row = layout.isOutShift() ^ (index.y % 2 == 0); // this row is on the left side of up/down row
    const int udleft_xidx = index.x - is_left_row - 1;                // x index of the up/down-left MI
    const int udright_xidx = udleft_xidx + 3;                         // x index of the up/down-right MI

    if (index.y > 0) {
        const int yidx = index.y - 1;
        if (udleft_xidx >= 0) {
            upleft_idx = {udleft_xidx, yidx};
            upleft_pt = layout.getMICenter(upleft_idx);
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            upright_idx = {udright_xidx, yidx};
            upright_pt = layout.getMICenter(upright_idx);
        }
    }

    if (index.y < (layout.getMIRows() - 1)) {
        const int yidx = index.y + 1;
        if (udleft_xidx >= 0) {
            downleft_idx = {udleft_xidx, yidx};
            downleft_pt = layout.getMICenter(downleft_idx);
        }
        if (udright_xidx < layout.getMICols(yidx)) {
            downright_idx = {udright_xidx, yidx};
            downright_pt = layout.getMICenter(downright_idx);
        }
    }

    const TIndices indices{upleft_idx, up_idx, upright_idx, downright_idx, down_idx, downleft_idx};
    const TPoints points{upleft_pt, up_pt, upright_pt, downright_pt, down_pt, downleft_pt};

    return {index, indices, self_pt, points};
}

using Neighbors = Neighbors_<tcfg::Layout>;

} // namespace tlct::cvt::raytrix::_hp