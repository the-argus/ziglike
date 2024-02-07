#include "test_header.h"
// test header must be first
#include "ziglike/defer.h"

using namespace zl;

TEST_SUITE("defer")
{
    TEST_CASE("construction")
    {
        SUBCASE("defer that does nothing")
        {
            defer([]() {});
        }
    }
}
