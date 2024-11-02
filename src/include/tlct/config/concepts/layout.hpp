#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/concepts/calib.hpp"
#include "tlct/config/concepts/specific.hpp"

namespace tlct::_cfg::concepts {

template <typename Self>
concept CLayout = std::is_trivially_copyable_v<Self> && requires {
    { Self::IS_KEPLER } -> std::convertible_to<bool>;
} && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires(const Self::TCalibConfig& calib_cfg, const Self::TSpecificConfig& spec_cfg) {
    // Init from
    requires CCalibConfig<typename Self::TCalibConfig>;
    requires CSpecificConfig<typename Self::TSpecificConfig>;
    { Self::fromCalibAndSpecConfig(calib_cfg, spec_cfg) } -> std::same_as<Self>;
} && requires(Self self) {
    // Non-const methods
    requires requires(int factor) {
        { self.upsample(factor) } noexcept -> std::same_as<Self&>;
    };
} && requires(const Self self) {
    // Const methods
    { self.getImgWidth() } noexcept -> std::integral;
    { self.getImgHeight() } noexcept -> std::integral;
    { self.getImgSize() } noexcept -> std::same_as<cv::Size>;
    { self.getDiameter() } noexcept -> std::floating_point;
    { self.getRadius() } noexcept -> std::floating_point;
    { self.getRotation() } noexcept -> std::floating_point;
    { self.getUpsample() } noexcept -> std::integral;
    { self.getMIRows() } noexcept -> std::integral;
    requires requires(int row) {
        { self.getMICols(row) } noexcept -> std::integral;
    };
    { self.getMIMaxCols() } noexcept -> std::integral;
    { self.getMIMinCols() } noexcept -> std::integral;
    requires requires(int row, int col) {
        { self.getMICenter(row, col) } noexcept -> std::same_as<cv::Point2d>;
    };
    requires requires(cv::Point index) {
        { self.getMICenter(index) } noexcept -> std::same_as<cv::Point2d>;
    };
    { self.isOutShift() } noexcept -> std::same_as<bool>;
    { self.isOutShiftSgn() } noexcept -> std::integral;

    requires requires(const cv::Mat& src, cv::Mat& dst) { self.processInto(src, dst); };
};

} // namespace tlct::_cfg::concepts
