#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/reader.hpp"
#endif

namespace tlct::_io::yuv {

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

template class YuvReader_<Yuv420Frame>;

}  // namespace tlct::_io::yuv
