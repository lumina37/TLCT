#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/concepts/layout.hpp"
#include "tlct/convert/helper/direction.hpp"

namespace tlct::cvt::concepts {

namespace tcfg = tlct::cfg;

template <typename Self>
concept CNeighbors = requires {
    // Initialize from
    requires requires(const Self::TLayout& layout, const cv::Point index) {
        tcfg::concepts::CLayout<typename Self::TLayout>;
        { Self::fromLayoutAndIndex(layout, index) } noexcept -> std::same_as<Self>;
    };
} && requires {
    requires requires(const Self& self) {
        { self.getSelfIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getSelfPt() } noexcept -> std::same_as<cv::Point2d>;

        { self.hasNeighbor(_hp::Direction::LEFT) } noexcept -> std::same_as<bool>;
        { self.getNeighborIdx(_hp::Direction::LEFT) } noexcept -> std::same_as<cv::Point>;
        { self.getNeighborPt(_hp::Direction::LEFT) } noexcept -> std::same_as<cv::Point2d>;
        { self.getUnitShift(_hp::Direction::LEFT) } noexcept -> std::same_as<cv::Point2d>;
    };
};

} // namespace tlct::cvt::concepts