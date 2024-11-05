#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/io/yuvp/frame.hpp"

namespace tlct {

namespace _io::yuvp {

namespace fs = std::filesystem;

template <typename TFrame_>
class YuvpReader_
{
public:
    using TFrame = TFrame_;

    TLCT_API inline YuvpReader_(std::ifstream&& ifs, size_t ywidth, size_t yheight)
        : ifs_(std::move(ifs)), ywidth_(ywidth), yheight_(yheight), ysize_(ywidth * yheight){};
    TLCT_API static inline YuvpReader_ fromPath(const fs::path& fpath, size_t ywidth, size_t yheight)
    {
        std::ifstream ifs{fpath, std::ios::binary};
        return {std::move(ifs), ywidth, yheight};
    }

    [[nodiscard]] TLCT_API inline size_t getYWidth() const noexcept { return ywidth_; };
    [[nodiscard]] TLCT_API inline size_t getYHeight() const noexcept { return yheight_; };
    [[nodiscard]] TLCT_API inline size_t getUWidth() const noexcept { return ywidth_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API inline size_t getUHeight() const noexcept { return yheight_ >> TFrame::Ushift; };
    [[nodiscard]] TLCT_API inline size_t getVWidth() const noexcept { return ywidth_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API inline size_t getVHeight() const noexcept { return yheight_ >> TFrame::Vshift; };
    [[nodiscard]] TLCT_API inline size_t getYSize() const noexcept { return ysize_; };
    [[nodiscard]] TLCT_API inline size_t getUSize() const noexcept { return ysize_ >> (TFrame::Ushift * 2); };
    [[nodiscard]] TLCT_API inline size_t getVSize() const noexcept { return ysize_ >> (TFrame::Vshift * 2); };
    [[nodiscard]] TLCT_API inline size_t getTotalSize() const noexcept
    {
        const size_t total_size = ysize_ + getUSize() + getVSize();
        return total_size;
    };

    TLCT_API inline YuvpReader_& skip(int n);
    TLCT_API inline TFrame poll();
    TLCT_API inline void poll_into(TFrame& frame);

private:
    std::ifstream ifs_;
    size_t ywidth_;
    size_t yheight_;
    size_t ysize_;
};

template <typename TFrame>
YuvpReader_<TFrame>& YuvpReader_<TFrame>::skip(int n)
{
    ifs_.seekg(n * getTotalSize());
    return *this;
}

template <typename TFrame>
TFrame YuvpReader_<TFrame>::poll()
{
    TFrame frame{getYWidth(), getYHeight()};
    ifs_.read((char*)frame.yptr_, getYSize());
    ifs_.read((char*)frame.uptr_, getUSize());
    ifs_.read((char*)frame.vptr_, getVSize());
    return frame;
}

template <typename TFrame>
void YuvpReader_<TFrame>::poll_into(TFrame& frame)
{
    ifs_.read((char*)frame.yptr_, getYSize());
    ifs_.read((char*)frame.uptr_, getUSize());
    ifs_.read((char*)frame.vptr_, getVSize());
}

using Yuv420pReader = YuvpReader_<Yuv420pFrame>;
template class YuvpReader_<Yuv420pFrame>;

} // namespace _io::yuvp

namespace io::yuvp {

namespace _ = _io::yuvp;

using _::Yuv420pReader;

} // namespace io::yuvp

} // namespace tlct
