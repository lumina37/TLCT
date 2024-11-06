#pragma once

#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"
#include "tlct/io/yuv/reader.hpp"
#include "tlct/io/yuv/writer.hpp"

namespace tlct {

namespace _io::yuv {

using Yuv420pFrame = YuvFrame_<uint8_t, 1, 1>;
static_assert(concepts::CFrame<Yuv420pFrame>);
template class YuvFrame_<uint8_t, 1, 1>;

using Yuv420pReader = YuvReader_<Yuv420pFrame>;
template class YuvReader_<Yuv420pFrame>;

using Yuv420pWriter = YuvWriter_<Yuv420pFrame>;
template class YuvWriter_<Yuv420pFrame>;

} // namespace _io::yuv

namespace io::yuv {

namespace _ = _io::yuv;

using _::Yuv420pFrame;
using _::Yuv420pReader;
using _::Yuv420pWriter;

} // namespace io::yuv

} // namespace tlct
