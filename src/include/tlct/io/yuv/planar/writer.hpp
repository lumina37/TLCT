#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/helper/error.hpp"
#include "tlct/io/yuv/planar/frame.hpp"
#include "tlct/helper/std.hpp"

namespace tlct::_io {

namespace fs = std::filesystem;

class YuvPlanarWriter {
    TLCT_API YuvPlanarWriter(std::ofstream&& ofs) noexcept;

public:
    [[nodiscard]] TLCT_API static std::expected<YuvPlanarWriter, Error> create(const fs::path& fpath) noexcept;

    [[nodiscard]] TLCT_API std::expected<void, Error> write(YuvPlanarFrame& frame) noexcept;

private:
    std::ofstream ofs_;
};

}  // namespace tlct::_io

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/writer.cpp"
#endif
