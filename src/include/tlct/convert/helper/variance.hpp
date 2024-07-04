#pragma once

#include <numeric>
#include <vector>

#include "tlct/common/defines.h"

namespace tlct::cvt::_hp {

TLCT_API inline double var_d(const std::vector<double>& vec)
{
    const double sum = std::reduce(vec.begin(), vec.end());
    const double avg = sum / (double)vec.size();

    double var = 0.0;
    for (const double elem : vec) {
        const double diff = elem - avg;
        var += diff * diff / (double)vec.size();
    }

    return var;
}

} // namespace tlct::cvt::_hp
