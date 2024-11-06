#pragma once

#include <filesystem>
#include <fstream>

#include "tlct/common/defines.h"
#include "tlct/helper/constexpr/math.hpp"
#include "tlct/io/yuv/frame.hpp"

namespace tlct {

namespace _io::yuv {

namespace fs = std::filesystem;

template <typename TFrame_>
class YuvWriter_
{
public:
    using TFrame = TFrame_;

    TLCT_API inline explicit YuvWriter_(std::ofstream&& ofs) : ofs_(std::move(ofs)){};
    TLCT_API static inline YuvWriter_ fromPath(const fs::path& fpath)
    {
        std::ofstream ofs{fpath, std::ios::binary};
        return YuvWriter_{std::move(ofs)};
    }

    TLCT_API inline void write(TFrame& frame);

private:
    std::ofstream ofs_;
};

template <typename TFrame>
void YuvWriter_<TFrame>::write(TFrame& frame)
{
    ofs_.write((char*)frame.getY().data, frame.getYSize());
    ofs_.write((char*)frame.getU().data, frame.getUSize());
    ofs_.write((char*)frame.getV().data, frame.getVSize());
}

using Yuv420pWriter = YuvWriter_<Yuv420pFrame>;
template class YuvWriter_<Yuv420pFrame>;

} // namespace _io::yuv

namespace io::yuv {

namespace _ = _io::yuv;

using _::Yuv420pWriter;

} // namespace io::yuv

} // namespace tlct
