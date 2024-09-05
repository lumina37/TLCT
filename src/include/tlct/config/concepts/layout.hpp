#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "calib.hpp"

namespace tlct::_cfg::concepts {

template <typename Self>
concept CLayout = std::copyable<Self> && requires {
    // Constructor
    { Self() } -> std::same_as<Self>;
} && requires(const Self::TParamConfig& cfg) {
    // Init from
    { Self::fromParamConfig(cfg) } -> std::same_as<Self>;
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
    { self.getRotation() } noexcept -> std::floating_point;
    { self.getUpsample() } noexcept -> std::integral;
    requires requires(int row, int col) {
        { self.getMICenter(row, col) } noexcept -> std::same_as<cv::Point2d>;
    };
    requires requires(cv::Point index) {
        { self.getMICenter(index) } noexcept -> std::same_as<cv::Point2d>;
    };
    { self.getMIRows() } noexcept -> std::integral;
    requires requires(int row) {
        { self.getMICols(row) } noexcept -> std::integral;
    };
    { self.getMIMaxCols() } noexcept -> std::integral;
    { self.getMIMinCols() } noexcept -> std::integral;
    { self.isOutShift() } noexcept -> std::same_as<bool>;
    { self.isOutShiftSgn() } noexcept -> std::integral;

    requires requires(const cv::Mat& src, cv::Mat& dst) { self.procImg_(src, dst); };
    requires requires(const cv::Mat& src) {
        { self.procImg(src) } -> std::same_as<cv::Mat>;
    };
};

} // namespace tlct::_cfg::concepts
