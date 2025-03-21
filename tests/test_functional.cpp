#include <array>

#include <catch2/catch_test_macros.hpp>

#include "tlct/convert/helper/functional.hpp"

TEST_CASE("Functional", "tlct::_cvt#pickByFWHM") {
    std::array arr = {3.f, 1.f, 3.f, 3.f, 2.f, 4.f};
    const int peak = tlct::_cvt::pickByFWHM(arr);
    REQUIRE(peak == 4);
}
