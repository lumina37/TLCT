#pragma once

#include <cmath>

namespace tlct::_hp {

class MeanStddev
{
public:
    inline MeanStddev() noexcept : mean_(), var_(), count_() {};

    inline void update(float val) noexcept
    {
        count_++;
        const float prev_mean = mean_;
        mean_ += (val - prev_mean) / (float)count_;
        var_ += (val - mean_) * (val - prev_mean);
    }

    [[nodiscard]] inline float getMean() const noexcept { return mean_; }
    [[nodiscard]] inline float getStddev() const noexcept { return sqrt(var_ / count_); }

private:
    float mean_;
    float var_;
    int count_;
};

} // namespace tlct::_hp
