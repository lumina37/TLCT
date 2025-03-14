#pragma once

#include <cstdint>

#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"
#include "tlct/io/yuv/reader.hpp"
#include "tlct/io/yuv/writer.hpp"

namespace tlct::io::yuv {

namespace _ = _io::yuv;

using _::YuvFrame_;
using _::YuvReader_;
using _::YuvWriter_;

using _::Yuv420Frame;
using _::Yuv420Reader;
using _::Yuv420Writer;

}  // namespace tlct::io::yuv
