#include <ios>
#include <sstream>
#include <stdexcept>

#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/writer.hpp"
#endif

namespace tlct::_io::yuv {

template <concepts::CFrame TFrame>
YuvWriter_<TFrame> YuvWriter_<TFrame>::fromPath(const fs::path& fpath) {
    std::ofstream ofs{fpath, std::ios::binary};
    if (!ofs) [[unlikely]] {
        std::stringstream err;
        err << "Failed to open file with 'w' mode. path: " << fpath;
        throw std::runtime_error{err.str()};
    }
    return YuvWriter_{std::move(ofs)};
}

template <concepts::CFrame TFrame>
void YuvWriter_<TFrame>::write(TFrame& frame) {
    ofs_.write((char*)frame.getY().data, frame.getYSize());
    ofs_.write((char*)frame.getU().data, frame.getUSize());
    ofs_.write((char*)frame.getV().data, frame.getVSize());
}

template class YuvWriter_<Yuv420Frame>;

}  // namespace tlct::_io::yuv
