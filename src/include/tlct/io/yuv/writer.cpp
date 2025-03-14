#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"

#ifndef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/writer.hpp"
#endif

namespace tlct::_io::yuv {

template <concepts::CFrame TFrame>
void YuvWriter_<TFrame>::write(TFrame& frame) {
    ofs_.write((char*)frame.getY().data, frame.getYSize());
    ofs_.write((char*)frame.getU().data, frame.getUSize());
    ofs_.write((char*)frame.getV().data, frame.getVSize());
}

template class YuvWriter_<Yuv420Frame>;

}  // namespace tlct::_io::yuv
