#pragma once

#include <concepts>

#include <opencv2/core.hpp>
#include <pugixml.hpp>

template <typename Self>
concept CCalibConfig = std::copyable<Self> && requires {
    // Constructors
    { Self() } -> std::same_as<Self>;
} && requires(const pugi::xml_document& doc) {
    // Init from `pugi::xml_document`
    { Self::fromXMLDoc(doc) } -> std::same_as<Self>;
} && requires(const char* path) {
    // Init from c-string
    { Self::fromXMLPath(path) } -> std::same_as<Self>;
};

template <typename Self>
concept CLayout = std::copyable<Self> && requires {
    // Constructors
    { Self() } -> std::same_as<Self>;
} && requires(const Self::TCalibConfig& cfg, cv::Size imgsize) {
    // Init from `Self::TCalibConfig`
    CCalibConfig<typename Self::TCalibConfig>;
    { Self::fromCfgAndImgsize(cfg, imgsize) } -> std::same_as<Self>;
} && requires(Self self, int factor) {
    // Non-const member functions
    { self.upsample(factor) } noexcept -> std::same_as<Self&>;
    { self.transpose() } -> std::same_as<Self&>;
} && requires(const Self self) {
    // Const member functions
    { self.getImgWidth() } noexcept -> std::integral;
    { self.getImgHeight() } noexcept -> std::integral;
    { self.getImgSize() } noexcept -> std::convertible_to<cv::Size>;
    { self.getDiameter() } noexcept -> std::floating_point;
    { self.getRotation() } noexcept -> std::floating_point;
    { self.getUpsample() } noexcept -> std::integral;
    requires requires(int row, int col) {
        { self.getMICenter(row, col) } noexcept -> std::convertible_to<cv::Point2d>;
    };
    requires requires(cv::Point index) {
        { self.getMICenter(index) } noexcept -> std::convertible_to<cv::Point2d>;
    };
    { self.getMIRows() } noexcept -> std::integral;
    requires requires(int row) {
        { self.getMICols(row) } noexcept -> std::integral;
    };
    { self.getMIMaxCols() } noexcept -> std::integral;
    { self.getMIMinCols() } noexcept -> std::integral;
    { self.isOutShift() } noexcept -> std::convertible_to<bool>;
    { self.isOutShiftSgn() } noexcept -> std::integral;
};
