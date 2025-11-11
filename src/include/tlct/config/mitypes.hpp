#pragma once

#include <array>

#include <opencv2/core.hpp>

#include "tlct/helper/std.hpp"

namespace tlct::_cfg {

class MITypes {
public:
    static constexpr int LEN_TYPE_NUM = 3;

    // Type alias
    using TIdx2Type = std::array<std::array<int, LEN_TYPE_NUM>, 2>;

    // Constructor
    TLCT_API MITypes() noexcept : idx2type_() {}
    MITypes(const MITypes& rhs) noexcept = default;
    MITypes& operator=(const MITypes& rhs) noexcept = default;
    MITypes(MITypes&& rhs) noexcept = default;
    MITypes& operator=(MITypes&& rhs) noexcept = default;
    TLCT_API MITypes(bool isOutShift) noexcept;

    [[nodiscard]] TLCT_API int getMIType(int row, int col) const noexcept;
    [[nodiscard]] TLCT_API int getMIType(cv::Point index) const noexcept;

private:
    TIdx2Type idx2type_;
};

}  // namespace tlct::_cfg

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/mitypes.cpp"
#endif
