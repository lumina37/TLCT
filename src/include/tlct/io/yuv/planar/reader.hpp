#pragma once

#include <expected>
#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/error.hpp"
#include "tlct/io/yuv/planar/extent.hpp"
#include "tlct/io/yuv/planar/frame.hpp"

namespace tlct::_io {

namespace fs = std::filesystem;

class YuvPlanarReader {
    YuvPlanarReader(std::ifstream&& ifs, const YuvPlanarExtent& extent) noexcept;

public:
    [[nodiscard]] TLCT_API static std::expected<YuvPlanarReader, Error> create(const fs::path& fpath,
                                                                               const YuvPlanarExtent& extent) noexcept;

    [[nodiscard]] TLCT_API std::expected<void, Error> skip(int frameCount) noexcept;
    [[nodiscard]] TLCT_API std::expected<YuvPlanarFrame, Error> read() noexcept;
    [[nodiscard]] TLCT_API std::expected<void, Error> readInto(YuvPlanarFrame& frame) noexcept;

private:
    std::ifstream ifs_;
    YuvPlanarExtent extent_;
};

}  // namespace tlct::_io

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/reader.cpp"
#endif
