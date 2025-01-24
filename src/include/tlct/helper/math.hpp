#pragma once

#include <cmath>

namespace tlct::_hp {

class MeanStddev {
public:
    inline MeanStddev() noexcept : mean_(), var_(), count_() {};

    inline void update(float val) noexcept {
        count_++;
        const float prevMean = mean_;
        mean_ += (val - prevMean) / (float)count_;
        var_ += (val - mean_) * (val - prevMean);
    }

    [[nodiscard]] inline float getMean() const noexcept { return mean_; }
    [[nodiscard]] inline float getStddev() const noexcept { return sqrtf(var_ / (float)count_); }

private:
    float mean_;
    float var_;
    int count_;
};

}  // namespace tlct::_hp
