#pragma once

#include "tlct/config/common.hpp"
#include "tlct/config/concepts.hpp"
#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_cvt::ssim {

template <cfg::concepts::CArrange TArrange_>
class PsizeParams_ {
public:
    static constexpr int INVALID_PSIZE = 0;

    // Typename alias
    using TArrange = TArrange_;
    using TCvtConfig = cfg::CliConfig::Convert;

    // Initialize from
    [[nodiscard]] TLCT_API static std::expected<PsizeParams_, Error> create(const TArrange& arrange,
                                                                            const TCvtConfig& cvtCfg) noexcept;

    float patternSize;
    float patternShift;
    int minPsize;
    float psizeShortcutThreshold;
};

}  // namespace tlct::_cvt::ssim

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/convert/patchsize/ssim/params.cpp"
#endif
