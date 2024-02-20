#include "test_header.h"
// test header must be first
#include "ziglike/stdmem.h"
#include "ziglike/zigstdint.h"
#include <array>

using namespace zl;

TEST_SUITE("stdmem")
{
    TEST_CASE("functions")
    {
        SUBCASE("invalid arguments")
        {
            std::array<u8, 512> bytes;

            slice<u8> a(bytes, 0, 100);
            slice<u8> b(bytes, 20, 110);

            REQUIRE(!memcopy(a, b));
            REQUIRE(!memcopy_lenient(b, a));
            // normally okay to copy into the bigger buffer, but these overlap
            REQUIRE(!memcopy_lenient(a, b));

            slice<u8> c(bytes, 200, 250);
            REQUIRE(memcopy_lenient(b, c));
            REQUIRE(memcopy_lenient(a, c));
            // c is smallest so you cant copy stuff into it
            REQUIRE(!memcopy_lenient(c, b));
            REQUIRE(!memcopy_lenient(c, a));
        }

        SUBCASE("memfill")
        {
            std::array<u8, 512> bytes;
            memfill(slice<u8>(bytes), u8(0));
            for (u8 byte : bytes) {
                REQUIRE(byte == 0);
            }

            memfill(slice<u8>(bytes, 0, 100), u8(1));
            for (size_t i = 0; i < bytes.size(); ++i) {
                REQUIRE(bytes[i] == (i < 100 ? 1 : 0));
            }
        }
    }
}
