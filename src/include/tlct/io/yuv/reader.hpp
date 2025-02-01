#pragma once

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <utility>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/io/concepts/frame.hpp"

namespace tlct::_io::yuv {

namespace fs = std::filesystem;

template <concepts::CFrame TFrame_>
class YuvReader_ {
public:
    using TFrame = TFrame_;

    TLCT_API inline YuvReader_(std::ifstream&& ifs, size_t yWidth, size_t yHeight)
        : ifs_(std::move(ifs)), yWidth_(yWidth), yHeight_(yHeight), ySize_(yWidth * yHeight){};
    TLCT_API static inline YuvReader_ fromPath(const fs::path& fpath, size_t yWidth, size_t yHeight) {
        std::ifstream ifs{fpath, std::ios::binary};
        return {std::move(ifs), yWidth, yHeight};
    }

    [[nodiscard]] TLCT_API inline size_t getYWidth() const noexcept { return yWidth_; };
    [[nodiscard]] TLCT_API inline size_t getYHeight() const noexcept { return yHeight_; };
    [[nodiscard]] TLCT_API inline size_t getUWidth() const noexcept { return yWidth_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API inline size_t getUHeight() const noexcept { return yHeight_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API inline size_t getVWidth() const noexcept { return yWidth_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API inline size_t getVHeight() const noexcept { return yHeight_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API inline size_t getYSize() const noexcept { return ySize_; };
    [[nodiscard]] TLCT_API inline size_t getUSize() const noexcept { return ySize_ >> (TFrame::Ushift * 2); };
    [[nodiscard]] TLCT_API inline size_t getVSize() const noexcept { return ySize_ >> (TFrame::Vshift * 2); };
    [[nodiscard]] TLCT_API inline size_t getTotalSize() const noexcept {
        const size_t totalSize = ySize_ + getUSize() + getVSize();
        return totalSize;
    };

    TLCT_API inline YuvReader_& skip(int n);
    TLCT_API inline TFrame read();
    TLCT_API inline void readInto(TFrame& frame);

private:
    std::ifstream ifs_;
    size_t yWidth_;
    size_t yHeight_;
    size_t ySize_;
};

template <concepts::CFrame TFrame>
YuvReader_<TFrame>& YuvReader_<TFrame>::skip(int n) {
    ifs_.seekg(n * getTotalSize());
    return *this;
}

template <concepts::CFrame TFrame>
TFrame YuvReader_<TFrame>::read() {
    TFrame frame{getYWidth(), getYHeight()};
    ifs_.read((char*)frame.getY().data, getYSize());
    ifs_.read((char*)frame.getU().data, getUSize());
    ifs_.read((char*)frame.getV().data, getVSize());
    return frame;
}

template <concepts::CFrame TFrame>
void YuvReader_<TFrame>::readInto(TFrame& frame) {
    ifs_.read((char*)frame.getY().data, getYSize());
    ifs_.read((char*)frame.getU().data, getUSize());
    ifs_.read((char*)frame.getV().data, getVSize());
}

}  // namespace tlct::_io::yuv
