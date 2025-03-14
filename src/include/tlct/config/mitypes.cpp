#include <ranges>

#include <opencv2/core.hpp>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/mitypes.hpp"
#endif

namespace tlct {

namespace _cfg {

namespace rgs = std::ranges;

MITypes::MITypes(bool isOutShift) noexcept {
    for (const int type : rgs::views::iota(0, LEN_TYPE_NUM)) {
        idx2type_[0][type] = type;
    }
    for (const int idx : rgs::views::iota(0, LEN_TYPE_NUM)) {
        const int type = idx2type_[0][(idx + 2 - isOutShift) % 3];
        idx2type_[1][idx] = type;
    }
}

int MITypes::getMIType(int row, int col) const noexcept {
    const int type = idx2type_[row % idx2type_.size()][col % LEN_TYPE_NUM];
    return type;
}

int MITypes::getMIType(cv::Point index) const noexcept { return getMIType(index.y, index.x); }

}  // namespace _cfg

namespace cfg {

using _cfg::MITypes;

}  // namespace cfg

}  // namespace tlct
