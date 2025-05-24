#include <expected>
#include <format>
#include <numbers>
#include <utility>

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/config/common/cli.hpp"
#endif

namespace tlct::_cfg {

CliConfig::CliConfig(Path&& path, const Range& range, const Convert& convert) noexcept
    : path(std::move(path)), range(range), convert(convert) {}

std::expected<CliConfig, Error> CliConfig::create(const Path& path, const Range& range,
                                                  const Convert& convert) noexcept {
    if (range.end <= range.begin) [[unlikely]] {
        auto errMsg = std::format("Expect range.end > range.begin, got: {} <= {}", range.end, range.begin);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.views <= 0) [[unlikely]] {
        auto errMsg = std::format("Expect views > 0, got: {}", convert.views);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.upsample <= 0) [[unlikely]] {
        auto errMsg = std::format("Expect upsample > 0, got: {}", convert.upsample);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.minPsize <= 0.0f || convert.minPsize >= 1.0f) [[unlikely]] {
        auto errMsg = std::format("Expect 0 < minPsize < 1, got: {}", convert.minPsize);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.psizeInflate < std::numbers::sqrt3_v<float> || convert.psizeInflate > 3.0f) [[unlikely]] {
        auto errMsg = std::format("Expect sqrt3 <= psizeInflate <= 3, got: {}", convert.psizeInflate);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.viewShiftRange < 0.0f || convert.viewShiftRange > 1.0f) [[unlikely]] {
        auto errMsg = std::format("Expect 0 <= viewShiftRange <= 1, got: {}", convert.viewShiftRange);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    if (convert.psizeShortcutFactor < 0.0f) [[unlikely]] {
        auto errMsg = std::format("Expect psizeShortcutFactor >= 0, got: {}", convert.psizeShortcutFactor);
        return std::unexpected{Error{ErrCode::InvalidParam, errMsg}};
    }

    auto copiedPath = path;
    return CliConfig{std::move(copiedPath), range, convert};
}

}  // namespace tlct::_cfg
