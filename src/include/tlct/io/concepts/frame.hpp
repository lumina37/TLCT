#pragma once

#include <concepts>

#include <opencv2/core.hpp>

namespace tlct::_io::concepts {

template <typename Self>
concept CFrame = requires {
    { Self::Ushift } -> std::convertible_to<size_t>;
    { Self::Vshift } -> std::convertible_to<size_t>;
} && requires {
    // Constructor
    requires requires(size_t ywidth, size_t yheight, size_t ysize) {
        { Self(ywidth, yheight, ysize) } -> std::same_as<Self>;
    } && requires(size_t ywidth, size_t yheight) {
        { Self(ywidth, yheight) } -> std::same_as<Self>;
    } && requires(const cv::Size& size) {
        { Self(size) } -> std::same_as<Self>;
    };
} && requires(Self self) {
    // Non-const methods
    { self.getY() } noexcept -> std::same_as<cv::Mat&>;
    { self.getU() } noexcept -> std::same_as<cv::Mat&>;
    { self.getV() } noexcept -> std::same_as<cv::Mat&>;
} && requires(const Self self) {
    // Const methods
    { self.getYWidth() } noexcept -> std::same_as<size_t>;
    { self.getYHeight() } noexcept -> std::same_as<size_t>;
    { self.getUWidth() } noexcept -> std::same_as<size_t>;
    { self.getUHeight() } noexcept -> std::same_as<size_t>;
    { self.getVWidth() } noexcept -> std::same_as<size_t>;
    { self.getVHeight() } noexcept -> std::same_as<size_t>;
    { self.getYSize() } noexcept -> std::same_as<size_t>;
    { self.getUSize() } noexcept -> std::same_as<size_t>;
    { self.getVSize() } noexcept -> std::same_as<size_t>;
    { self.getTotalSize() } noexcept -> std::same_as<size_t>;

    { self.getY() } noexcept -> std::same_as<const cv::Mat&>;
    { self.getU() } noexcept -> std::same_as<const cv::Mat&>;
    { self.getV() } noexcept -> std::same_as<const cv::Mat&>;
};

} // namespace tlct::_io::concepts
