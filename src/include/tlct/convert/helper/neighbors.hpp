#pragma once

#include <array>
#include <cstddef>
#include <numbers>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

template <tlct::cfg::concepts::CArrange TArrange_>
class NearNeighbors_
{
public:
    enum class Direction {
        UPLEFT,
        UPRIGHT,
        LEFT,
        RIGHT,
        DOWNLEFT,
        DOWNRIGHT,
        COUNT,
    };
    static constexpr int DIRECTION_NUM = (int)Direction::COUNT;

    static constexpr float INFLATE = 1.0;
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr float DEFAULT_COORD = -1.0;

    // Typename alias
    using TArrange = TArrange_;
    using TIndices = std::array<cv::Point, DIRECTION_NUM>;
    using TPoints = std::array<cv::Point2f, DIRECTION_NUM>;
    using TUnitShifts = std::array<std::array<float, 2>, DIRECTION_NUM>;

    static constexpr float SCALAR_UNIT_SHIFT = 1.0;
    static constexpr float X_UNIT_STEP = 0.5 * SCALAR_UNIT_SHIFT;
    static constexpr float Y_UNIT_STEP = std::numbers::sqrt3 / 2.0 * SCALAR_UNIT_SHIFT;

    static constexpr TUnitShifts UNIT_SHIFTS{{
        {-X_UNIT_STEP, -Y_UNIT_STEP},
        {X_UNIT_STEP, -Y_UNIT_STEP},
        {-SCALAR_UNIT_SHIFT, 0.0},
        {SCALAR_UNIT_SHIFT, 0.0},
        {-X_UNIT_STEP, Y_UNIT_STEP},
        {X_UNIT_STEP, Y_UNIT_STEP},
    }};

    // Constructor
    NearNeighbors_() = delete;
    TLCT_API inline NearNeighbors_(const NearNeighbors_& rhs) noexcept = default;
    TLCT_API inline NearNeighbors_& operator=(const NearNeighbors_& rhs) noexcept = default;
    TLCT_API inline NearNeighbors_(NearNeighbors_&& rhs) noexcept = default;
    TLCT_API inline NearNeighbors_& operator=(NearNeighbors_&& rhs) noexcept = default;
    TLCT_API inline NearNeighbors_(TIndices indices, cv::Point self_idx, TPoints points, cv::Point2f self_pt) noexcept
        : indices_(indices), self_idx_(self_idx), points_(points), self_pt_(self_pt){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline NearNeighbors_ fromArrangeAndIndex(const TArrange& arrange,
                                                                            cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Point getSelfIdx() const noexcept { return self_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point2f getSelfPt() const noexcept { return self_pt_; };

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API inline bool hasNeighbor(const Direction direction) const noexcept
    {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    };
    [[nodiscard]] TLCT_API inline cv::Point getNeighborIdx(const Direction direction) const noexcept
    {
        return indices_[(int)direction];
    };
    [[nodiscard]] TLCT_API inline cv::Point2f getNeighborPt(const Direction direction) const noexcept
    {
        return points_[(int)direction];
    };
    [[nodiscard]] TLCT_API static inline cv::Point2f getUnitShift(const Direction direction) noexcept
    {
        const auto& unit_shift = UNIT_SHIFTS[(int)direction];
        return {unit_shift[0], unit_shift[1]};
    };

private:
    TIndices indices_;
    cv::Point self_idx_;
    TPoints points_;
    cv::Point2f self_pt_;

private:
    template <size_t... Idx>
    [[nodiscard]] static consteval auto make_directions(std::index_sequence<Idx...>)
    {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array<Direction, DIRECTION_NUM> DIRECTIONS{
        make_directions(std::make_index_sequence<DIRECTION_NUM>{})};
};

template <tlct::cfg::concepts::CArrange TArrange>
NearNeighbors_<TArrange> NearNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange,
                                                                       cv::Point index) noexcept
{
    cv::Point left_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point right_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point upleft_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point upright_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point downleft_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point downright_idx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};

    cv::Point2f self_pt = arrange.getMICenter(index);
    cv::Point2f left_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f right_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f upleft_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f upright_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f downleft_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f downright_pt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};

    if (index.x > 0) [[likely]] {
        left_idx = {index.x - 1, index.y};
        left_pt = arrange.getMICenter(left_idx);
    }
    if (index.x < (arrange.getMICols(index.y) - 1)) [[likely]] {
        right_idx = {index.x + 1, index.y};
        right_pt = arrange.getMICenter(right_idx);
    }

    const int is_left_row = arrange.isOutShift() ^ (index.y % 2 == 0); // this row is on the left side of up/down row
    const int udleft_xidx = index.x - is_left_row;                     // x index of the up/down-left MI
    const int udright_xidx = udleft_xidx + 1;                          // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yidx = index.y - 1;
        if (udleft_xidx >= 0) [[likely]] {
            upleft_idx = {udleft_xidx, yidx};
            upleft_pt = arrange.getMICenter(upleft_idx);
        }
        if (udright_xidx < arrange.getMICols(yidx)) [[likely]] {
            upright_idx = {udright_xidx, yidx};
            upright_pt = arrange.getMICenter(upright_idx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yidx = index.y + 1;
        if (udleft_xidx >= 0) [[likely]] {
            downleft_idx = {udleft_xidx, yidx};
            downleft_pt = arrange.getMICenter(downleft_idx);
        }
        if (udright_xidx < arrange.getMICols(yidx)) [[likely]] {
            downright_idx = {udright_xidx, yidx};
            downright_pt = arrange.getMICenter(downright_idx);
        }
    }

    const TIndices indices{upleft_idx, upright_idx, left_idx, right_idx, downleft_idx, downright_idx};
    const TPoints points{upleft_pt, upright_pt, left_pt, right_pt, downleft_pt, downright_pt};

    return {indices, index, points, self_pt};
}

template <tlct::cfg::concepts::CArrange TArrange_>
class FarNeighbors_
{
public:
    enum class Direction {
        UP,
        UPLEFT,
        UPRIGHT,
        DOWNLEFT,
        DOWNRIGHT,
        DOWN,
        COUNT,
    };
    static constexpr int DIRECTION_NUM = (int)Direction::COUNT;

    static constexpr float INFLATE = std::numbers::sqrt3;
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr float DEFAULT_COORD = -1.0;

    // Typename alias
    using TArrange = TArrange_;
    using TIndices = std::array<cv::Point, DIRECTION_NUM>;
    using TPoints = std::array<cv::Point2f, DIRECTION_NUM>;
    using TUnitShifts = std::array<std::array<float, 2>, DIRECTION_NUM>;

    static constexpr float SCALAR_UNIT_SHIFT = 1.0;
    static constexpr float X_UNIT_STEP = std::numbers::sqrt3 / 2.0 * SCALAR_UNIT_SHIFT;
    static constexpr float Y_UNIT_STEP = 0.5 * SCALAR_UNIT_SHIFT;

    static constexpr TUnitShifts UNIT_SHIFTS{{
        {0.0, -SCALAR_UNIT_SHIFT},
        {-X_UNIT_STEP, -Y_UNIT_STEP},
        {X_UNIT_STEP, -Y_UNIT_STEP},
        {-X_UNIT_STEP, Y_UNIT_STEP},
        {X_UNIT_STEP, Y_UNIT_STEP},
        {0.0, SCALAR_UNIT_SHIFT},
    }};

    // Constructor
    FarNeighbors_() = delete;
    TLCT_API inline FarNeighbors_(const FarNeighbors_& rhs) noexcept = default;
    TLCT_API inline FarNeighbors_& operator=(const FarNeighbors_& rhs) noexcept = default;
    TLCT_API inline FarNeighbors_(FarNeighbors_&& rhs) noexcept = default;
    TLCT_API inline FarNeighbors_& operator=(FarNeighbors_&& rhs) noexcept = default;
    TLCT_API inline FarNeighbors_(TIndices indices, cv::Point self_idx, TPoints points, cv::Point2f self_pt) noexcept
        : indices_(indices), self_idx_(self_idx), points_(points), self_pt_(self_pt){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline FarNeighbors_ fromArrangeAndIndex(const TArrange& arrange,
                                                                           cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Point getSelfIdx() const noexcept { return self_idx_; };
    [[nodiscard]] TLCT_API inline cv::Point2f getSelfPt() const noexcept { return self_pt_; };

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API inline bool hasNeighbor(const Direction direction) const noexcept
    {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    };
    [[nodiscard]] TLCT_API inline cv::Point getNeighborIdx(const Direction direction) const noexcept
    {
        return indices_[(int)direction];
    };
    [[nodiscard]] TLCT_API inline cv::Point2f getNeighborPt(const Direction direction) const noexcept
    {
        return points_[(int)direction];
    };
    [[nodiscard]] TLCT_API static inline cv::Point2f getUnitShift(const Direction direction) noexcept
    {
        const auto& unit_shift = UNIT_SHIFTS[(int)direction];
        return {unit_shift[0], unit_shift[1]};
    };

private:
    TIndices indices_;
    cv::Point self_idx_;
    TPoints points_;
    cv::Point2f self_pt_;

private:
    template <size_t... Idx>
    [[nodiscard]] static consteval auto make_directions(std::index_sequence<Idx...>)
    {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array<Direction, DIRECTION_NUM> DIRECTIONS{
        make_directions(std::make_index_sequence<DIRECTION_NUM>{})};
};

template <tlct::cfg::concepts::CArrange TArrange>
FarNeighbors_<TArrange> FarNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange, cv::Point index) noexcept
{
    cv::Point up_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point down_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point upleft_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point upright_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point downleft_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point downright_idx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};

    cv::Point2f self_pt = arrange.getMICenter(index);
    cv::Point2f up_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f down_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f upleft_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f upright_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f downleft_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f downright_pt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};

    const int is_left_row = arrange.isOutShift() ^ (index.y % 2 == 0); // this row is on the left side of up/down row
    const int udleft_xidx = index.x - is_left_row - 1;                 // x index of the up/down-left MI
    const int udright_xidx = udleft_xidx + 3;                          // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yidx = index.y - 1;
        if (udleft_xidx >= 0) [[likely]] {
            upleft_idx = {udleft_xidx, yidx};
            upleft_pt = arrange.getMICenter(upleft_idx);
        }
        if (udright_xidx < arrange.getMICols(yidx)) [[likely]] {
            upright_idx = {udright_xidx, yidx};
            upright_pt = arrange.getMICenter(upright_idx);
        }

        if (index.y > 1) [[likely]] {
            up_idx = {index.x, index.y - 2};
            up_pt = arrange.getMICenter(up_idx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yidx = index.y + 1;
        if (udleft_xidx >= 0) [[likely]] {
            downleft_idx = {udleft_xidx, yidx};
            downleft_pt = arrange.getMICenter(downleft_idx);
        }
        if (udright_xidx < arrange.getMICols(yidx)) [[likely]] {
            downright_idx = {udright_xidx, yidx};
            downright_pt = arrange.getMICenter(downright_idx);
        }

        if (index.y < (arrange.getMIRows() - 2)) [[likely]] {
            down_idx = {index.x, index.y + 2};
            down_pt = arrange.getMICenter(down_idx);
        }
    }

    const TIndices indices{up_idx, upleft_idx, upright_idx, downleft_idx, downright_idx, down_idx};
    const TPoints points{up_pt, upleft_pt, upright_pt, downleft_pt, downright_pt, down_pt};

    return {indices, index, points, self_pt};
}

} // namespace tlct::_cvt
