#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"

namespace tlct::_cvt::concepts {

template <typename Self>
concept CNeighbors = requires {
    // Constant
    { Self::INFLATE } -> std::convertible_to<float>;
} && requires {
    // Initialize from
    requires requires(const typename Self::TArrange& arrange, const cv::Point index) {
        requires cfg::concepts::CArrange<typename Self::TArrange>;
        { Self::fromArrangeAndIndex(arrange, index) } noexcept -> std::same_as<Self>;
    };
} && requires {
    requires requires(const Self& self) {
        { self.getSelfIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getSelfPt() } noexcept -> std::same_as<cv::Point2f>;

        { self.hasNeighbor((typename Self::Direction)0) } noexcept -> std::same_as<bool>;
        { self.getNeighborIdx((typename Self::Direction)0) } noexcept -> std::same_as<cv::Point>;
        { self.getNeighborPt((typename Self::Direction)0) } noexcept -> std::same_as<cv::Point2f>;
        { self.getUnitShift((typename Self::Direction)0) } noexcept -> std::same_as<cv::Point2f>;
    };
};

}  // namespace tlct::_cvt::concepts
