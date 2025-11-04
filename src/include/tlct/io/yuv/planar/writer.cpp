#include <expected>
#include <format>
#include <ios>

#include "tlct/io/yuv/planar/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/writer.hpp"
#endif

namespace tlct::_io {

YuvPlanarWriter::YuvPlanarWriter(std::ofstream&& ofs) noexcept : ofs_(std::move(ofs)) {}

std::expected<YuvPlanarWriter, Error> YuvPlanarWriter::create(const fs::path& fpath) noexcept {
    std::ofstream ofs{fpath, std::ios::binary};
    if (!ofs.good()) [[unlikely]] {
        auto errMsg =
            std::format("failed to open read-only file. path={}, iostate={}", fpath.string(), (int)ofs.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }
    return YuvPlanarWriter{std::move(ofs)};
}

std::expected<void, Error> YuvPlanarWriter::write(YuvPlanarFrame& frame) noexcept {
    ofs_.write((char*)frame.getY().data, frame.getExtent().getYByteSize());
    if (!ofs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to write. iostate={}", (int)ofs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ofs_.write((char*)frame.getU().data, frame.getExtent().getUByteSize());
    if (!ofs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to write. iostate={}", (int)ofs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ofs_.write((char*)frame.getV().data, frame.getExtent().getVByteSize());
    if (!ofs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to write. iostate={}", (int)ofs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    return {};
}

}  // namespace tlct::_io
