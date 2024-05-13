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
                static_assert(std::is_same_v<decltype(item), const int>);
                static_assert(std::is_same_v<decltype(index), const size_t>);

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

        SUBCASE("enumerate moved vector")
        {
            std::vector<uint8_t> mem;
            mem.reserve(500);
            for (size_t i = 0; i < 500; ++i) {
                mem.push_back(0);
            }

            size_t i = 0;
            for (auto [item, index] : enumerate(std::move(mem))) {
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
            zl::slice<uint8_t> test(mem);
            for (auto [item, index] : enumerate(test)) {
                REQUIRE(item == 0);
                REQUIRE(index == i);
                ++i;
            }
        }

        SUBCASE("enumerate const vector of large type")
        {
            struct Test
            {
                int i;
                size_t j;
            };
            std::vector<Test> mem;
            mem.reserve(500);
            for (size_t i = 0; i < 500; ++i) {
                mem.push_back({});
            }

            const std::vector<Test> &memref = mem;

            size_t i = 0;
            for (auto [item, index] : enumerate(memref)) {
                static_assert(std::is_same_v<decltype(item), const Test &>);
                REQUIRE(item.i == 0);
                REQUIRE(item.j == 0);
                REQUIRE(index == i);
                ++i;
            }
        }

        SUBCASE("enumerate is by value when size is smaller than reference")
        {
            std::array<uint8_t, 500> mem = {0};

            size_t i = 0;
            for (auto [item, index] : enumerate(mem)) {
                static_assert(std::is_same_v<decltype(item), const uint8_t>);
                REQUIRE(item == 0);
                REQUIRE(index == i);
                ++i;
            }
        }

        SUBCASE("use enumerate_mut to avoid iterating by value")
        {
            std::array<uint8_t, 256> mem = {0};

            size_t i = 0;
            for (auto [item, index] : enumerate_mut(mem)) {
                static_assert(std::is_same_v<decltype(item), uint8_t &>);
                REQUIRE(item == 0);
                REQUIRE(index == i);
                item = index;
                ++i;
            }

            for (auto [item, index] : enumerate(mem)) {
                REQUIRE(item == index);
            }
        }
    }
}
