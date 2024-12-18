#pragma once

#include <array>
#include <ranges>

#include <opencv2/core.hpp>

#include "tlct/common/defines.h"

namespace tlct {

namespace _cfg {

namespace rgs = std::ranges;

class MITypes
{
public:
    static constexpr int LEN_TYPE_NUM = 3;

    // Type alias
    using TIdx2Type = std::array<std::array<int, LEN_TYPE_NUM>, 2>;

    // Constructor
    TLCT_API inline MITypes() noexcept : idx2type_(){};
    TLCT_API inline MITypes(const MITypes& rhs) noexcept = default;
    TLCT_API inline MITypes& operator=(const MITypes& rhs) noexcept = default;
    TLCT_API inline MITypes(MITypes&& rhs) noexcept = default;
    TLCT_API inline MITypes& operator=(MITypes&& rhs) noexcept = default;
    TLCT_API inline MITypes(bool is_out_shift) noexcept;

    [[nodiscard]] TLCT_API inline int getMIType(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API inline int getMIType(cv::Point index) const noexcept;

private:
    TIdx2Type idx2type_;
};

MITypes::MITypes(bool is_out_shift) noexcept
{
    for (const int type : rgs::views::iota(0, LEN_TYPE_NUM)) {
        idx2type_[0][type] = type;
    }
    for (const int idx : rgs::views::iota(0, LEN_TYPE_NUM)) {
        const int type = idx2type_[0][(idx + 2 - is_out_shift) % 3];
        idx2type_[1][idx] = type;
    }
}

int MITypes::getMIType(int row, int col) const noexcept
{
    const int type = idx2type_[row % idx2type_.size()][col % LEN_TYPE_NUM];
    return type;
}

int MITypes::getMIType(cv::Point index) const noexcept { return getMIType(index.y, index.x); }

} // namespace _cfg

namespace cfg {

using _cfg::MITypes;

} // namespace cfg

} // namespace tlct
