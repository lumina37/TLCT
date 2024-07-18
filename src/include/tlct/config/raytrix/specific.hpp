#pragma once

#include "calib.hpp"

#include <opencv2/core.hpp>

namespace tlct::_cfg::raytrix {

class SpecificConfig
{
public:
    static constexpr double DEFAULT_PATTERN_SIZE = 0.325;
    static constexpr double DEFAULT_GRADIENT_BLENDING_WIDTH = 0.225;
    static constexpr double DEFAULT_SAFE_RANGE = 1.0 - DEFAULT_PATTERN_SIZE * DEFAULT_GRADIENT_BLENDING_WIDTH;
    static constexpr double DEFAULT_PSIZE_SHORTCUT_THRESHOLD = -0.875;

    // Constructor
    TLCT_API inline SpecificConfig() noexcept
        : imgsize_(), upsample_(1), pattern_size_(DEFAULT_PATTERN_SIZE),
          gradient_blending_width_(DEFAULT_GRADIENT_BLENDING_WIDTH), safe_range_(DEFAULT_SAFE_RANGE),
          psize_shortcut_threshold_(DEFAULT_PSIZE_SHORTCUT_THRESHOLD){};
    TLCT_API inline SpecificConfig& operator=(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig& operator=(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(const cv::Size imgsize, int upsample, double pattern_size,
                                   double gradient_blending_width, double psize_shortcut_threshold) noexcept
        : imgsize_(imgsize), upsample_(upsample), pattern_size_(pattern_size),
          gradient_blending_width_(gradient_blending_width),
          safe_range_(std::min((1.0 - pattern_size), 1.0 / (1.0 + gradient_blending_width))),
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
    const int width = cfg_map.get<int>("width");
    const int height = cfg_map.get<int>("height");
    const int upsample = cfg_map.get<int>("upsample");
    const double kernel_size = cfg_map.get("patternSize", DEFAULT_PATTERN_SIZE);
    const double gradient_blending_width = cfg_map.get("gradientBlendingWidth", DEFAULT_GRADIENT_BLENDING_WIDTH);
    const double psize_shortcut_threshold = cfg_map.get("psizeShortcutThreshold", DEFAULT_GRADIENT_BLENDING_WIDTH);
    return {{width, height}, upsample, kernel_size, gradient_blending_width, psize_shortcut_threshold};
}

} // namespace tlct::_cfg::raytrix
