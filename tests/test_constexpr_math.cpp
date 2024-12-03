#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace hp = tlct::_hp;

TEST_CASE("tlct::_hp#constexpr_math")
{
    // int round
    CHECK(hp::iround(3737.1) == 3737);
    CHECK(hp::iround(3737.6) == 3738);

    // round to
    CHECK(hp::roundTo<8>(17) == 16);
    CHECK(hp::roundTo<8>(14) == 16);

    // is pow of 2
    CHECK(hp::isPowOf2(14u) == false);
    CHECK(hp::isPowOf2(16u) == true);

    // is multiple of
    CHECK(hp::isMulOf<4>(16u) == true);
    CHECK(hp::isMulOf<4>(9u) == false);

    // align up
    CHECK(hp::alignUp<64>(125) == 128);
    CHECK(hp::alignUp<64>(128) == 128);

    // align down
    CHECK(hp::alignDown<64>(130) == 128);
    CHECK(hp::alignDown<64>(128) == 128);

    // sign
    CHECK(hp::sgn(true) == 1);
    CHECK(hp::sgn(false) == -1);
}
