#pragma once

#include <concepts>
#include <filesystem>

#include <opencv2/core.hpp>

#include <tlct/config/common/map.hpp>

namespace tlct::_cfg::concepts {

namespace fs = std::filesystem;

template <typename Self>
concept CArrange = std::is_trivially_copyable_v<Self> && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires(const ConfigMap& map) {
    // Init from
    { Self::fromCfgMap(map) } -> std::same_as<Self>;
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
    { self.getRawImgSize() } noexcept -> std::same_as<cv::Size>;
    { self.getDiameter() } noexcept -> std::floating_point;
    { self.getRadius() } noexcept -> std::floating_point;
    { self.getDirection() } noexcept -> std::same_as<bool>;
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
};

} // namespace tlct::_cfg::concepts
