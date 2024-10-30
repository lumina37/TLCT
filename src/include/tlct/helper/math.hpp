#pragma once

#include <cmath>

namespace tlct::_hp {

class MeanStddev
{
public:
    inline MeanStddev() noexcept : mean_(), var_(), count_() {};

    inline void update(double val) noexcept
    {
        count_++;
        const double prev_mean = mean_;
        mean_ += (val - prev_mean) / (float)count_;
        var_ += (val - mean_) * (val - prev_mean);
    }

    [[nodiscard]] inline double getMean() const noexcept { return mean_; }
    [[nodiscard]] inline double getStddev() const noexcept { return sqrt(var_ / count_); }

private:
    double mean_;
    double var_;
    int count_;
};

} // namespace tlct::_hp
