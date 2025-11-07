#include <format>
#include <ios>

#include "tlct/helper/error.hpp"
#include "tlct/helper/std.hpp"
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
        auto errMsg = std::format("failed to open read-only file. path={}", fpath.string());
        return std::unexpected{Error{ECate::eSys, ifs.rdstate(), std::move(errMsg)}};
    }

    return YuvPlanarReader{std::move(ifs), extent};
}

std::expected<void, Error> YuvPlanarReader::skip(int frameCount) noexcept {
    ifs_.seekg(frameCount * extent_.getTotalByteSize());

    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to skip {} frames", frameCount);
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    return {};
}

std::expected<YuvPlanarFrame, Error> YuvPlanarReader::read() noexcept {
    auto frameRes = YuvPlanarFrame::create(extent_);
    if (!frameRes) [[unlikely]] {
        return frameRes;
    }
    auto& frame = frameRes.value();

    ifs_.read((char*)frame.getY().data, extent_.getYByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    ifs_.read((char*)frame.getU().data, extent_.getUByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    ifs_.read((char*)frame.getV().data, extent_.getVByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    return std::move(frame);
}

std::expected<void, Error> YuvPlanarReader::readInto(YuvPlanarFrame& frame) noexcept {
    ifs_.read((char*)frame.getY().data, frame.getExtent().getYByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    ifs_.read((char*)frame.getU().data, frame.getExtent().getUByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    ifs_.read((char*)frame.getV().data, frame.getExtent().getVByteSize());
    if (!ifs_.good()) [[unlikely]] {
        auto errMsg = std::format("failed to read");
        return std::unexpected{Error{ECate::eSys, ifs_.rdstate(), std::move(errMsg)}};
    }

    return {};
}

}  // namespace tlct::_io
