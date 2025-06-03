#pragma once

#include <vector>

namespace tlct::_cvt {

class PatchRecord {
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
