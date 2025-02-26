#include <catch2/catch_test_macros.hpp>

#include "tlct.hpp"

namespace hp = tlct::_hp;

TEST_CASE("Compile-time math", "tlct::_hp#constexpr_math") {
    // int round
    REQUIRE(hp::iround(3737.1) == 3737);
    REQUIRE(hp::iround(3737.6) == 3738);

    // round to
    REQUIRE(hp::roundTo<8>(17) == 16);
    REQUIRE(hp::roundTo<8>(14) == 16);

    // is pow of 2
    REQUIRE(hp::isPowOf2(14u) == false);
    REQUIRE(hp::isPowOf2(16u) == true);

    // is multiple of
    REQUIRE(hp::isMulOf<4>(16u) == true);
    REQUIRE(hp::isMulOf<4>(9u) == false);

    // align up
    REQUIRE(hp::alignUp<64>(125) == 128);
    REQUIRE(hp::alignUp<64>(128) == 128);

    // align down
    REQUIRE(hp::alignDown<64>(130) == 128);
    REQUIRE(hp::alignDown<64>(128) == 128);

    // sign
    REQUIRE(hp::sgn(true) == 1);
    REQUIRE(hp::sgn(false) == -1);
}
