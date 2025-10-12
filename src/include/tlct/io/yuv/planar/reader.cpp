#include <expected>
#include <format>
#include <ios>
#include <utility>

#include "tlct/helper/error.hpp"
#include "tlct/io/yuv/planar/extent.hpp"
#include "tlct/io/yuv/planar/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/planar/reader.hpp"
#endif

namespace tlct::_io {

YuvPlanarReader::YuvPlanarReader(std::ifstream&& ifs, const YuvPlanarExtent& extent) noexcept
    : ifs_(std::move(ifs)), extent_(extent) {}

std::expected<YuvPlanarReader, Error> YuvPlanarReader::create(const fs::path& fpath,
                                                              const YuvPlanarExtent& extent) noexcept {
    std::ifstream ifs{fpath, std::ios::binary};
    if (!ifs.good()) [[unlikely]] {
        auto errMsg = std::format("failed to open read-only file. path={}, iostate={}", fpath.string(), (int)ifs.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    return YuvPlanarReader{std::move(ifs), extent};
}

std::expected<void, Error> YuvPlanarReader::skip(int frameCount) noexcept {
    ifs_.seekg(frameCount * extent_.getTotalSize());

    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to skip {} frames. iostate={}", frameCount, (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    return {};
}

std::expected<YuvPlanarFrame, Error> YuvPlanarReader::read() noexcept {
    auto frameRes = YuvPlanarFrame::create(extent_);
    if (!frameRes) [[unlikely]] {
        return frameRes;
    }
    auto& frame = frameRes.value();

    ifs_.read((char*)frame.getY().data, extent_.getYSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ifs_.read((char*)frame.getU().data, extent_.getUSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ifs_.read((char*)frame.getV().data, extent_.getVSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    return std::move(frame);
}

std::expected<void, Error> YuvPlanarReader::readInto(YuvPlanarFrame& frame) noexcept {
    ifs_.read((char*)frame.getY().data, frame.getExtent().getYSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ifs_.read((char*)frame.getU().data, frame.getExtent().getUSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    ifs_.read((char*)frame.getV().data, frame.getExtent().getVSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read. iostate={}", (int)ifs_.rdstate());
        return std::unexpected{Error{ErrCode::FileSysError, errMsg}};
    }

    return {};
}

}  // namespace tlct::_io
