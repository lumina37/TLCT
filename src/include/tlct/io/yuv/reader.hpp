#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <utility>

#include "tlct/common/defines.h"
#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"

namespace tlct::_io::yuv {

namespace fs = std::filesystem;

template <concepts::CFrame TFrame_>
class YuvReader_ {
public:
    using TFrame = TFrame_;

    TLCT_API YuvReader_(std::ifstream&& ifs, size_t yWidth, size_t yHeight)
        : ifs_(std::move(ifs)), yWidth_(yWidth), yHeight_(yHeight), ySize_(yWidth * yHeight) {};
    TLCT_API static YuvReader_ fromPath(const fs::path& fpath, size_t yWidth, size_t yHeight);

    [[nodiscard]] TLCT_API size_t getYWidth() const noexcept { return yWidth_; };
    [[nodiscard]] TLCT_API size_t getYHeight() const noexcept { return yHeight_; };
    [[nodiscard]] TLCT_API size_t getUWidth() const noexcept { return yWidth_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API size_t getUHeight() const noexcept { return yHeight_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API size_t getVWidth() const noexcept { return yWidth_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API size_t getVHeight() const noexcept { return yHeight_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API size_t getYSize() const noexcept { return ySize_; };
    [[nodiscard]] TLCT_API size_t getUSize() const noexcept { return ySize_ >> (TFrame::Ushift * 2); };
    [[nodiscard]] TLCT_API size_t getVSize() const noexcept { return ySize_ >> (TFrame::Vshift * 2); };
    [[nodiscard]] TLCT_API size_t getTotalSize() const noexcept {
        const size_t totalSize = ySize_ + getUSize() + getVSize();
        return totalSize;
    };

    TLCT_API YuvReader_& skip(int n);
    TLCT_API TFrame read();
    TLCT_API void readInto(TFrame& frame);

private:
    std::ifstream ifs_;
    size_t yWidth_;
    size_t yHeight_;
    size_t ySize_;
};

using Yuv420Reader = YuvReader_<Yuv420Frame>;

}  // namespace tlct::_io::yuv

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/reader.cpp"
#endif
