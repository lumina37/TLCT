#pragma once

#include <array>
#include <cstddef>
#include <numbers>
#include <utility>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

template <cfg::concepts::CArrange TArrange_>
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
    NearNeighbors_(const NearNeighbors_& rhs) noexcept = default;
    NearNeighbors_& operator=(const NearNeighbors_& rhs) noexcept = default;
    NearNeighbors_(NearNeighbors_&& rhs) noexcept = default;
    NearNeighbors_& operator=(NearNeighbors_&& rhs) noexcept = default;
    TLCT_API NearNeighbors_(TIndices indices, cv::Point selfIdx, TPoints points, cv::Point2f selfPt) noexcept
        : indices_(indices), selfIdx_(selfIdx), points_(points), selfPt_(selfPt) {}

    // Initialize from
    [[nodiscard]] TLCT_API static NearNeighbors_ fromArrangeAndIndex(const TArrange& arrange, cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Point getSelfIdx() const noexcept { return selfIdx_; }
    [[nodiscard]] TLCT_API cv::Point2f getSelfPt() const noexcept { return selfPt_; }

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API bool hasNeighbor(const Direction direction) const noexcept {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    }
    [[nodiscard]] TLCT_API cv::Point getNeighborIdx(const Direction direction) const noexcept {
        return indices_[(int)direction];
    }
    [[nodiscard]] TLCT_API cv::Point2f getNeighborPt(const Direction direction) const noexcept {
        return points_[(int)direction];
    }
    [[nodiscard]] TLCT_API static cv::Point2f getUnitShift(const Direction direction) noexcept {
        const auto& unitShift = UNIT_SHIFTS[(int)direction];
        return {unitShift[0], unitShift[1]};
    }

private:
    TIndices indices_;
    cv::Point selfIdx_;
    TPoints points_;
    cv::Point2f selfPt_;

private:
    template <size_t... Idx>
    [[nodiscard]] static consteval auto makeDirections(std::index_sequence<Idx...>) noexcept {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array DIRECTIONS{makeDirections(std::make_index_sequence<DIRECTION_NUM>{})};
};

template <cfg::concepts::CArrange TArrange_>
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
    FarNeighbors_(const FarNeighbors_& rhs) noexcept = default;
    FarNeighbors_& operator=(const FarNeighbors_& rhs) noexcept = default;
    FarNeighbors_(FarNeighbors_&& rhs) noexcept = default;
    FarNeighbors_& operator=(FarNeighbors_&& rhs) noexcept = default;
    TLCT_API FarNeighbors_(TIndices indices, cv::Point selfIdx, TPoints points, cv::Point2f selfPt) noexcept
        : indices_(indices), selfIdx_(selfIdx), points_(points), selfPt_(selfPt) {}

    // Initialize from
    [[nodiscard]] TLCT_API static FarNeighbors_ fromArrangeAndIndex(const TArrange& arrange, cv::Point index) noexcept;

    // Const methods
    [[nodiscard]] TLCT_API cv::Point getSelfIdx() const noexcept { return selfIdx_; }
    [[nodiscard]] TLCT_API cv::Point2f getSelfPt() const noexcept { return selfPt_; }

    // Left is 0. Clockwise.
    [[nodiscard]] TLCT_API bool hasNeighbor(const Direction direction) const noexcept {
        return indices_[(int)direction].x != DEFAULT_INDEX;
    }
    [[nodiscard]] TLCT_API cv::Point getNeighborIdx(const Direction direction) const noexcept {
        return indices_[(int)direction];
    }
    [[nodiscard]] TLCT_API cv::Point2f getNeighborPt(const Direction direction) const noexcept {
        return points_[(int)direction];
    }
    [[nodiscard]] TLCT_API static cv::Point2f getUnitShift(const Direction direction) noexcept {
        const auto& unitShift = UNIT_SHIFTS[(int)direction];
        return {unitShift[0], unitShift[1]};
    }

private:
    TIndices indices_;
    cv::Point selfIdx_;
    TPoints points_;
    cv::Point2f selfPt_;

    template <size_t... Idx>
    [[nodiscard]] static consteval auto makeDirections(std::index_sequence<Idx...>) noexcept {
        return std::array<Direction, sizeof...(Idx)>{(Direction)Idx...};
    }

public:
    static constexpr std::array DIRECTIONS{makeDirections(std::make_index_sequence<DIRECTION_NUM>{})};
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/helper/neighbors.cpp"
#endif
