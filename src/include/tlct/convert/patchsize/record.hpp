#pragma once

#include <vector>

namespace tlct::_cvt {

template <bool DEBUG>
class PatchRecord_;

template <>
class PatchRecord_<false> {
public:
    float psize;
};

template <>
class PatchRecord_<true> {
public:
    float psize;
    bool isBlurredNear;
    bool isBlurredFar;
    bool estimatedByFar;
    uint16_t dhash;
    std::vector<float> nearMetrics;
    std::vector<float> farMetrics;
};

}  // namespace tlct::_cvt
