#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/io/concepts/frame.hpp"

namespace tlct::_io::yuv {

namespace fs = std::filesystem;

template <concepts::CFrame TFrame_>
class YuvReader_
{
public:
    using TFrame = TFrame_;

    TLCT_API inline YuvReader_(std::ifstream&& ifs, size_t ywidth, size_t yheight)
        : ifs_(std::move(ifs)), ywidth_(ywidth), yheight_(yheight), ysize_(ywidth * yheight){};
    TLCT_API static inline YuvReader_ fromPath(const fs::path& fpath, size_t ywidth, size_t yheight)
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

    TLCT_API inline YuvReader_& skip(int n);
    TLCT_API inline TFrame read();
    TLCT_API inline void read_into(TFrame& frame);

private:
    std::ifstream ifs_;
    size_t ywidth_;
    size_t yheight_;
    size_t ysize_;
};

template <concepts::CFrame TFrame>
YuvReader_<TFrame>& YuvReader_<TFrame>::skip(int n)
{
    ifs_.seekg(n * getTotalSize());
    return *this;
}

template <concepts::CFrame TFrame>
TFrame YuvReader_<TFrame>::read()
{
    TFrame frame{getYWidth(), getYHeight()};
    ifs_.read((char*)frame.getY().data, getYSize());
    ifs_.read((char*)frame.getU().data, getUSize());
    ifs_.read((char*)frame.getV().data, getVSize());
    return frame;
}

template <concepts::CFrame TFrame>
void YuvReader_<TFrame>::read_into(TFrame& frame)
{
    ifs_.read((char*)frame.getY().data, getYSize());
    ifs_.read((char*)frame.getU().data, getUSize());
    ifs_.read((char*)frame.getV().data, getVSize());
}

} // namespace tlct::_io::yuv
