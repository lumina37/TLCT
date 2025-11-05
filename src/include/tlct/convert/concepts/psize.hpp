#pragma once

#include <concepts>

#include <opencv2/core.hpp>

namespace tlct::_cvt::concepts {

template <typename Self>
concept CPsizeImpl = requires {
    // Const methods
    requires requires(Self self, int offset) {
        { self.getPatchsize(offset) } -> std::floating_point;
        { self.getWeight(offset) } -> std::floating_point;
    } && requires(Self self, int row, int col) {
        { self.getPatchsize(row, col) } -> std::floating_point;
        { self.getWeight(row, col) } -> std::floating_point;
    };
};

}  // namespace tlct::_cvt::concepts
