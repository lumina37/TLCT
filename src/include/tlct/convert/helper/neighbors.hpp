#pragma once

#include <array>
#include <cstddef>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

template <tlct::cfg::concepts::CArrange TArrange_>
class NearNeighbors_ {
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

    static constexpr float INFLATE = 1.f;
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr float DEFAULT_COORD = -1.f;

    // Typename alias
    using TArrange = TArrange_;
    using TIndices = std::array<cv::Point, DIRECTION_NUM>;
    using TPoints = std::array<cv::Point2f, DIRECTION_NUM>;
    using TUnitShifts = std::array<std::array<float, 2>, DIRECTION_NUM>;

    static constexpr float SCALAR_UNIT_SHIFT = 1.f;
    static constexpr float X_UNIT_STEP = 0.5 * SCALAR_UNIT_SHIFT;
    static constexpr float Y_UNIT_STEP = std::numbers::sqrt3_v<float> / 2.f * SCALAR_UNIT_SHIFT;

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
    TLCT_API NearNeighbors_(const NearNeighbors_& rhs) noexcept = default;
    TLCT_API NearNeighbors_& operator=(const NearNeighbors_& rhs) noexcept = default;
    TLCT_API NearNeighbors_(NearNeighbors_&& rhs) noexcept = default;
    TLCT_API NearNeighbors_& operator=(NearNeighbors_&& rhs) noexcept = default;
    TLCT_API NearNeighbors_(TIndices indices, cv::Point selfIdx, TPoints points, cv::Point2f selfPt) noexcept
        : indices_(indices), selfIdx_(selfIdx), points_(points), selfPt_(selfPt) {};

    // Initialize from
    [[nodiscard]] TLCT_API static NearNeighbors_ fromArrangeAndIndex(const TArrange& arrange, cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Point getSelfIdx() const noexcept { return selfIdx_; };
    [[nodiscard]] TLCT_API cv::Point2f getSelfPt() const noexcept { return selfPt_; };

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API bool hasNeighbor(const Direction direction) const noexcept {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    };
    [[nodiscard]] TLCT_API cv::Point getNeighborIdx(const Direction direction) const noexcept {
        return indices_[(int)direction];
    };
    [[nodiscard]] TLCT_API cv::Point2f getNeighborPt(const Direction direction) const noexcept {
        return points_[(int)direction];
    };
    [[nodiscard]] TLCT_API static cv::Point2f getUnitShift(const Direction direction) noexcept {
        const auto& unitShift = UNIT_SHIFTS[(int)direction];
        return {unitShift[0], unitShift[1]};
    };

private:
    TIndices indices_;
    cv::Point selfIdx_;
    TPoints points_;
    cv::Point2f selfPt_;

private:
    template <size_t... Idx>
    [[nodiscard]] static consteval auto makeDirections(std::index_sequence<Idx...>) {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array DIRECTIONS{makeDirections(std::make_index_sequence<DIRECTION_NUM>{})};
};

template <tlct::cfg::concepts::CArrange TArrange>
NearNeighbors_<TArrange> NearNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange,
                                                                       cv::Point index) noexcept {
    cv::Point leftIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point rightIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point upleftIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point uprightIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point downleftIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};
    cv::Point downrightIdx{NearNeighbors_::DEFAULT_INDEX, NearNeighbors_::DEFAULT_INDEX};

    cv::Point2f selfPt = arrange.getMICenter(index);
    cv::Point2f leftPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f rightPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f upleftPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f uprightPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f downleftPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};
    cv::Point2f downrightPt{NearNeighbors_::DEFAULT_COORD, NearNeighbors_::DEFAULT_COORD};

    if (index.x > 0) [[likely]] {
        leftIdx = {index.x - 1, index.y};
        leftPt = arrange.getMICenter(leftIdx);
    }
    if (index.x < (arrange.getMICols(index.y) - 1)) [[likely]] {
        rightIdx = {index.x + 1, index.y};
        rightPt = arrange.getMICenter(rightIdx);
    }

    const int isLeftRow = arrange.isOutShift() ^ (index.y % 2 == 0);  // this row is on the left side of up/down row
    const int udLeftXIdx = index.x - isLeftRow;                       // x index of the up/down-left MI
    const int udRightXIdx = udLeftXIdx + 1;                           // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yIdx = index.y - 1;
        if (udLeftXIdx >= 0) [[likely]] {
            upleftIdx = {udLeftXIdx, yIdx};
            upleftPt = arrange.getMICenter(upleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            uprightIdx = {udRightXIdx, yIdx};
            uprightPt = arrange.getMICenter(uprightIdx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yIdx = index.y + 1;
        if (udLeftXIdx >= 0) [[likely]] {
            downleftIdx = {udLeftXIdx, yIdx};
            downleftPt = arrange.getMICenter(downleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            downrightIdx = {udRightXIdx, yIdx};
            downrightPt = arrange.getMICenter(downrightIdx);
        }
    }

    const TIndices indices{upleftIdx, uprightIdx, leftIdx, rightIdx, downleftIdx, downrightIdx};
    const TPoints points{upleftPt, uprightPt, leftPt, rightPt, downleftPt, downrightPt};

    return {indices, index, points, selfPt};
}

template <tlct::cfg::concepts::CArrange TArrange_>
class FarNeighbors_ {
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

    static constexpr float INFLATE = std::numbers::sqrt3_v<float>;
    static constexpr int DEFAULT_INDEX = -1;
    static constexpr float DEFAULT_COORD = -1.f;

    // Typename alias
    using TArrange = TArrange_;
    using TIndices = std::array<cv::Point, DIRECTION_NUM>;
    using TPoints = std::array<cv::Point2f, DIRECTION_NUM>;
    using TUnitShifts = std::array<std::array<float, 2>, DIRECTION_NUM>;

    static constexpr float SCALAR_UNIT_SHIFT = 1.f;
    static constexpr float X_UNIT_STEP = std::numbers::sqrt3_v<float> / 2.f * SCALAR_UNIT_SHIFT;
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
    TLCT_API FarNeighbors_(const FarNeighbors_& rhs) noexcept = default;
    TLCT_API FarNeighbors_& operator=(const FarNeighbors_& rhs) noexcept = default;
    TLCT_API FarNeighbors_(FarNeighbors_&& rhs) noexcept = default;
    TLCT_API FarNeighbors_& operator=(FarNeighbors_&& rhs) noexcept = default;
    TLCT_API FarNeighbors_(TIndices indices, cv::Point selfIdx, TPoints points, cv::Point2f selfPt) noexcept
        : indices_(indices), selfIdx_(selfIdx), points_(points), selfPt_(selfPt) {};

    // Initialize from
    [[nodiscard]] TLCT_API static FarNeighbors_ fromArrangeAndIndex(const TArrange& arrange, cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Point getSelfIdx() const noexcept { return selfIdx_; };
    [[nodiscard]] TLCT_API cv::Point2f getSelfPt() const noexcept { return selfPt_; };

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API bool hasNeighbor(const Direction direction) const noexcept {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    };
    [[nodiscard]] TLCT_API cv::Point getNeighborIdx(const Direction direction) const noexcept {
        return indices_[(int)direction];
    };
    [[nodiscard]] TLCT_API cv::Point2f getNeighborPt(const Direction direction) const noexcept {
        return points_[(int)direction];
    };
    [[nodiscard]] TLCT_API static cv::Point2f getUnitShift(const Direction direction) noexcept {
        const auto& unitShift = UNIT_SHIFTS[(int)direction];
        return {unitShift[0], unitShift[1]};
    };

private:
    TIndices indices_;
    cv::Point selfIdx_;
    TPoints points_;
    cv::Point2f selfPt_;

private:
    template <size_t... Idx>
    [[nodiscard]] static consteval auto makeDirections(std::index_sequence<Idx...>) {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array DIRECTIONS{makeDirections(std::make_index_sequence<DIRECTION_NUM>{})};
};

template <tlct::cfg::concepts::CArrange TArrange>
FarNeighbors_<TArrange> FarNeighbors_<TArrange>::fromArrangeAndIndex(const TArrange& arrange,
                                                                     cv::Point index) noexcept {
    cv::Point upIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point downIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point upleftIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point uprightIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point downleftIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};
    cv::Point downrightIdx{FarNeighbors_::DEFAULT_INDEX, FarNeighbors_::DEFAULT_INDEX};

    cv::Point2f selfPt = arrange.getMICenter(index);
    cv::Point2f upPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f downPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f upleftPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f uprightPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f downleftPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};
    cv::Point2f downrightPt{FarNeighbors_::DEFAULT_COORD, FarNeighbors_::DEFAULT_COORD};

    const int isLeftRow = arrange.isOutShift() ^ (index.y % 2 == 0);  // this row is on the left side of up/down row
    const int udLeftXIdx = index.x - isLeftRow - 1;                   // x index of the up/down-left MI
    const int udRightXIdx = udLeftXIdx + 3;                           // x index of the up/down-right MI

    if (index.y > 0) [[likely]] {
        const int yIdx = index.y - 1;
        if (udLeftXIdx >= 0) [[likely]] {
            upleftIdx = {udLeftXIdx, yIdx};
            upleftPt = arrange.getMICenter(upleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            uprightIdx = {udRightXIdx, yIdx};
            uprightPt = arrange.getMICenter(uprightIdx);
        }

        if (index.y > 1) [[likely]] {
            upIdx = {index.x, index.y - 2};
            upPt = arrange.getMICenter(upIdx);
        }
    }

    if (index.y < (arrange.getMIRows() - 1)) [[likely]] {
        const int yIdx = index.y + 1;
        if (udLeftXIdx >= 0) [[likely]] {
            downleftIdx = {udLeftXIdx, yIdx};
            downleftPt = arrange.getMICenter(downleftIdx);
        }
        if (udRightXIdx < arrange.getMICols(yIdx)) [[likely]] {
            downrightIdx = {udRightXIdx, yIdx};
            downrightPt = arrange.getMICenter(downrightIdx);
        }

        if (index.y < (arrange.getMIRows() - 2)) [[likely]] {
            downIdx = {index.x, index.y + 2};
            downPt = arrange.getMICenter(downIdx);
        }
    }

    const TIndices indices{upIdx, upleftIdx, uprightIdx, downleftIdx, downrightIdx, downIdx};
    const TPoints points{upPt, upleftPt, uprightPt, downleftPt, downrightPt, downPt};

    return {indices, index, points, selfPt};
}

}  // namespace tlct::_cvt
