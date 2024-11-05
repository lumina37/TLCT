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
class YuvpWriter_
{
public:
    using TFrame = TFrame_;

    TLCT_API inline YuvpWriter_(std::ofstream&& ofs) : ofs_(std::move(ofs)){};
    TLCT_API static inline YuvpWriter_ fromPath(const fs::path& fpath)
    {
        std::ofstream ofs{fpath, std::ios::binary};
        return YuvpWriter_{std::move(ofs)};
    }

    TLCT_API inline void write(TFrame& frame);

private:
    std::ofstream ofs_;
};

template <typename TFrame>
void YuvpWriter_<TFrame>::write(TFrame& frame)
{
    ofs_.write((char*)frame.yptr_, frame.getYSize());
    ofs_.write((char*)frame.uptr_, frame.getUSize());
    ofs_.write((char*)frame.vptr_, frame.getVSize());
}

using Yuv420pWriter = YuvpWriter_<Yuv420pFrame>;
template class YuvpWriter_<Yuv420pFrame>;

} // namespace _io::yuvp

namespace io::yuvp {

namespace _ = _io::yuvp;

using _::Yuv420pWriter;

} // namespace io::yuvp

} // namespace tlct
