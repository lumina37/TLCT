#pragma once

#include <filesystem>
#include <fstream>
#include <utility>

#include "tlct/common/defines.h"
#include "tlct/io/concepts/frame.hpp"
#include "tlct/io/yuv/frame.hpp"

namespace tlct::_io::yuv {

namespace fs = std::filesystem;

template <concepts::CFrame TFrame_>
class YuvWriter_ {
public:
    using TFrame = TFrame_;

    TLCT_API explicit YuvWriter_(std::ofstream&& ofs) : ofs_(std::move(ofs)){};
    TLCT_API static YuvWriter_ fromPath(const fs::path& fpath) {
        std::ofstream ofs{fpath, std::ios::binary};
        return YuvWriter_{std::move(ofs)};
    }

    TLCT_API void write(TFrame& frame);

private:
    std::ofstream ofs_;
};

using Yuv420Writer = YuvWriter_<Yuv420Frame>;

}  // namespace tlct::_io::yuv

#ifdef _TLCT_LIB_HEADER_ONLY
#    include "tlct/io/yuv/writer.cpp"
#endif
