#pragma once

#include <concepts>

#include <opencv2/core.hpp>

#include "tlct/config/concepts.hpp"
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
        { self.hasLeft() } noexcept -> std::same_as<bool>;
        { self.hasRight() } noexcept -> std::same_as<bool>;
        { self.hasUpLeft() } noexcept -> std::same_as<bool>;
        { self.hasUpRight() } noexcept -> std::same_as<bool>;
        { self.hasDownLeft() } noexcept -> std::same_as<bool>;
        { self.hasDownRight() } noexcept -> std::same_as<bool>;

        { self.getSelfIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getLeftIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getRightIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getUpLeftIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getUpRightIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getDownLeftIdx() } noexcept -> std::same_as<cv::Point>;
        { self.getDownRightIdx() } noexcept -> std::same_as<cv::Point>;
        { self.template getNeighborIdx<Direction::LEFT>() } noexcept -> std::same_as<cv::Point>;

        { self.getSelfPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getLeftPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getRightPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getUpLeftPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getUpRightPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getDownLeftPt() } noexcept -> std::same_as<cv::Point2d>;
        { self.getDownRightPt() } noexcept -> std::same_as<cv::Point2d>;

        { self.template hasNeighbor<Direction::LEFT>() } noexcept -> std::same_as<bool>;
        { self.template getNeighborIdx<Direction::LEFT>() } noexcept -> std::same_as<cv::Point>;
        { self.template getNeighborPt<Direction::LEFT>() } noexcept -> std::same_as<cv::Point2d>;
    };
};

} // namespace tlct::cvt::concepts