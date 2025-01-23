#pragma once

#include <cstdint>

namespace tlct::_cvt {

struct PsizeRecord {
    int64_t psize;
    uint64_t hash;
};

struct PsizeMetric {
    int psize;
    float metric;
};

} // namespace tlct::_cvt
