#pragma once

#include <expected>

#include "tlct/common/defines.h"
#include "tlct/helper/error.hpp"
#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"

namespace tlct::_cvt {

namespace tcfg = tlct::cfg;

template <tcfg::concepts::CArrange TArrange_>
class PsizeParams_ {
public:
    static constexpr int INVALID_PSIZE = 0;

    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = tcfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PsizeParams_, Error> create(const TArrange& arrange,
                                                                            const TCvtConfig& cvtCfg) noexcept;

    int minPsize;
    int maxPsize;
    float psizeShortcutFactor;
};

}  // namespace tlct::_cvt

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/params.cpp"
#endif
