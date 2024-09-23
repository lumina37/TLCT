#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "tlct.hpp"

namespace hp = tlct::_hp;

TEST_CASE("static_math")
{
    // int round
    CHECK(hp::iround(3737.1) == 3737);
    CHECK(hp::iround(3737.6) == 3738);

    // round to
    CHECK(hp::round_to<8>(17) == 16);
    CHECK(hp::round_to<8>(14) == 16);

    // is pow of 2
    CHECK(hp::is_pow_of_2(14u) == false);
    CHECK(hp::is_pow_of_2(16u) == true);

    // align up
    CHECK(hp::align_up<64>(125) == 128);
    CHECK(hp::align_up<64>(128) == 128);

    // align down
    CHECK(hp::align_down<64>(130) == 128);
    CHECK(hp::align_down<64>(128) == 128);

    // sign
    CHECK(hp::sgn(true) == 1);
    CHECK(hp::sgn(false) == -1);
}
