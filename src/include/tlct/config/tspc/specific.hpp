#pragma once

#include "calib.hpp"

#include <opencv2/core.hpp>

namespace tlct::cfg::tspc {

class SpecificConfig
{
public:
    // Constructor
    TLCT_API inline SpecificConfig() noexcept
        : imgsize_(), upsample_(1), pattern_size_(0.357), gradient_blending_width_(0.225),
          safe_range_(1.0 - 0.357 * 0.225), psize_shortcut_threshold_(-0.875){};
    TLCT_API inline SpecificConfig& operator=(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig& operator=(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(const cv::Size imgsize, int upsample, double pattern_size,
                                   double gradient_blending_width, double psize_shortcut_threshold) noexcept
        : imgsize_(imgsize), upsample_(upsample), pattern_size_(pattern_size),
          gradient_blending_width_(gradient_blending_width),
          safe_range_((1.0 - pattern_size) / (1.0 + gradient_blending_width)),
          psize_shortcut_threshold_(psize_shortcut_threshold){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline SpecificConfig fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline double getSafeRange() const noexcept { return safe_range_; };
    [[nodiscard]] TLCT_API inline double getPatternSize() const noexcept { return pattern_size_; };
    [[nodiscard]] TLCT_API inline double getGradientBlendingWidth() const noexcept { return gradient_blending_width_; };
    [[nodiscard]] TLCT_API inline double getPsizeShortcutThreshold() const noexcept
    {
        return psize_shortcut_threshold_;
    };

private:
    cv::Size imgsize_;
    int upsample_;
    double safe_range_;
    double pattern_size_;
    double gradient_blending_width_;
    double psize_shortcut_threshold_;
};

SpecificConfig SpecificConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const auto& map = cfg_map.getMap();
    const int width = std::stoi(map.at("width"));
    const int height = std::stoi(map.at("height"));
    const int upsample = std::stoi(map.at("upsample"));
    const double kernel_size = std::stod(map.at("patternSize"));
    const double gradient_blending_width = std::stod(map.at("gradientBlendingWidth"));
    const double psize_shortcut_threshold = std::stod(map.at("psizeShortcutThreshold"));
    return {{width, height}, upsample, kernel_size, gradient_blending_width, psize_shortcut_threshold};
}

} // namespace tlct::cfg::tspc
