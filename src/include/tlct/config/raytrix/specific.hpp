#pragma once

#include <numbers>

#include <opencv2/core.hpp>

#include "calib.hpp"
#include "tlct/common/defines.h"

namespace tlct::_cfg::raytrix {

class SpecificConfig
{
public:
    static constexpr int DEFAULT_UPSAMPLE = 1;
    static constexpr double DEFAULT_MAX_PATCH_SIZE = 0.75;
    static constexpr double DEFAULT_PATTERN_SIZE = 0.3;
    static constexpr double DEFAULT_GRADIENT_BLENDING_WIDTH = 0.45;
    static constexpr double DEFAULT_PSIZE_SHORTCUT_THRESHOLD = -0.9;

    static constexpr double PSIZE_AMP = std::numbers::sqrt3;

    // Constructor
    TLCT_API inline SpecificConfig() noexcept
        : imgsize_(), upsample_(1),
          max_patch_size_(std::min(DEFAULT_MAX_PATCH_SIZE, 1.0 / (1.0 + DEFAULT_GRADIENT_BLENDING_WIDTH) / PSIZE_AMP)),
          pattern_size_(DEFAULT_PATTERN_SIZE), gradient_blending_width_(DEFAULT_GRADIENT_BLENDING_WIDTH),
          psize_shortcut_threshold_(DEFAULT_PSIZE_SHORTCUT_THRESHOLD){};
    TLCT_API inline SpecificConfig(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig& operator=(const SpecificConfig& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig& operator=(SpecificConfig&& rhs) noexcept = default;
    TLCT_API inline SpecificConfig(cv::Size imgsize, int upsample, double max_patch_size, double pattern_size,
                                   double gradient_blending_width, double psize_shortcut_threshold) noexcept
        : imgsize_(imgsize), upsample_(upsample), max_patch_size_(std::min(max_patch_size, 1.0 / PSIZE_AMP)),
          pattern_size_(pattern_size), gradient_blending_width_(gradient_blending_width),
          psize_shortcut_threshold_(psize_shortcut_threshold){};

    // Initialize from
    [[nodiscard]] TLCT_API static inline SpecificConfig fromConfigMap(const ConfigMap& cfg_map);

    // Const methods
    [[nodiscard]] TLCT_API inline cv::Size getImgSize() const noexcept { return imgsize_; };
    [[nodiscard]] TLCT_API inline int getUpsample() const noexcept { return upsample_; };
    [[nodiscard]] TLCT_API inline double getMaxPatchSize() const noexcept { return max_patch_size_; };
    [[nodiscard]] TLCT_API inline double getPatternSize() const noexcept { return pattern_size_; };
    [[nodiscard]] TLCT_API inline double getGradientBlendingWidth() const noexcept { return gradient_blending_width_; };
    [[nodiscard]] TLCT_API inline double getPsizeShortcutThreshold() const noexcept
    {
        return psize_shortcut_threshold_;
    };

private:
    double max_patch_size_;
    double pattern_size_;
    double gradient_blending_width_;
    double psize_shortcut_threshold_;
    cv::Size imgsize_;
    int upsample_;
};

SpecificConfig SpecificConfig::fromConfigMap(const ConfigMap& cfg_map)
{
    const int width = cfg_map.get<int>("width");
    const int height = cfg_map.get<int>("height");
    const int upsample = cfg_map.get("upsample", DEFAULT_UPSAMPLE);
    const double max_patch_size = cfg_map.get("maxPatchSize", DEFAULT_MAX_PATCH_SIZE);
    const double pattern_size = cfg_map.get("patternSize", DEFAULT_PATTERN_SIZE);
    const double gradient_blending_width = cfg_map.get("gradientBlendingWidth", DEFAULT_GRADIENT_BLENDING_WIDTH);
    const double psize_shortcut_threshold = cfg_map.get("psizeShortcutThreshold", DEFAULT_PSIZE_SHORTCUT_THRESHOLD);
    return {{width, height}, upsample, max_patch_size, pattern_size, gradient_blending_width, psize_shortcut_threshold};
}

} // namespace tlct::_cfg::raytrix
