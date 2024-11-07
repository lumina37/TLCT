#pragma once

#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"
#include "tlct/io/yuv/reader.hpp"
#include "tlct/io/yuv/writer.hpp"

namespace tlct {

namespace _io::yuv {

using Yuv420Frame = YuvFrame_<uint8_t, 1, 1>;
static_assert(concepts::CFrame<Yuv420Frame>);
template class YuvFrame_<uint8_t, 1, 1>;

using Yuv420Reader = YuvReader_<Yuv420Frame>;
template class YuvReader_<Yuv420Frame>;

using Yuv420Writer = YuvWriter_<Yuv420Frame>;
template class YuvWriter_<Yuv420Frame>;

} // namespace _io::yuv

namespace io::yuv {

namespace _ = _io::yuv;

using _::YuvFrame_;
using _::YuvReader_;
using _::YuvWriter_;

using _::Yuv420Frame;
using _::Yuv420Reader;
using _::Yuv420Writer;

} // namespace io::yuv

} // namespace tlct
