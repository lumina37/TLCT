#pragma once

#include "tlct/io/concepts.hpp"
#include "tlct/io/yuv.hpp"

namespace tlct::io {

namespace _ = yuv;

using _::YuvFrame_;
using _::YuvReader_;
using _::YuvWriter_;

using _::Yuv420Frame;
using _::Yuv420Reader;
using _::Yuv420Writer;

} // namespace tlct::io
