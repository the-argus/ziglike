#include "test_header.h"
// test header must be first
#include "ziglike/enumerate.h"
#include "ziglike/slice.h"
#include <array>
#include <vector>

using namespace zl;

TEST_SUITE("enumerate")
{
    TEST_CASE("functionality")
    {
        SUBCASE("enumerate array")
        {
            std::array<int, 500> ints;
            std::fill(ints.begin(), ints.end(), 0);

            size_t i = 0;
            for (auto [item, index] : enumerate(ints)) {
                static_assert(std::is_same_v<decltype(item), int &>);
                static_assert(std::is_same_v<decltype(index), size_t>);

                REQUIRE(item == 0);
                REQUIRE(index == i);

                ++i;
            }
        }

        SUBCASE("enumerate vector")
        {
            std::vector<uint8_t> mem;
            mem.reserve(500);
            for (size_t i = 0; i < 500; ++i) {
                mem.push_back(0);
            }

            size_t i = 0;
            for (auto [item, index] : enumerate(mem)) {
                REQUIRE(item == 0);
                REQUIRE(index == i);
                ++i;
            }
        }

        SUBCASE("enumerate slice")
        {
            std::vector<uint8_t> mem;
            mem.reserve(500);
            for (size_t i = 0; i < 500; ++i) {
                mem.push_back(0);
            }

            size_t i = 0;
            for (auto [item, index] : enumerate(zl::slice<uint8_t>(mem))) {
                REQUIRE(item == 0);
                REQUIRE(index == i);
                ++i;
            }
        }

        // SUBCASE("enumerate const vector")
        // {
        //     std::vector<uint8_t> mem;
        //     mem.reserve(500);
        //     for (size_t i = 0; i < 500; ++i) {
        //         mem.push_back(0);
        //     }

        //     const std::vector<uint8_t>& memref = mem;

        //     size_t i = 0;
        //     for (auto [item, index] : enumerate(memref)) {
        //         REQUIRE(item == 0);
        //         REQUIRE(index == i);
        //         ++i;
        //     }
        // }
    }
}
