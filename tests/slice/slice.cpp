#include "test_header.h"
// test header must be first
#include "ziglike/detail/is_container.h"
#include "ziglike/enumerate.h"
#include "ziglike/slice.h"
#include <array>
#include <vector>

static_assert(
    zl::detail::is_container_v<std::array<int, 1>>,
    "is_container not properly detecting size() and data() functions");

static_assert(
    zl::detail::is_container_v<std::vector<int>>,
    "is_container not properly detecting size() and data() functions");

static_assert(!zl::detail::is_container_v<int>, "int registered as container?");

using namespace zl;

static_assert(std::is_same_v<slice<const uint8_t>::type, const uint8_t>,
              "slice::type doesnt work as expected");

TEST_SUITE("slice")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construction")
        {
            std::array<uint8_t, 512> mem;

            // unfortunately the template argument is not deducable
            slice<uint8_t> sl(mem);
            REQUIRE(sl.size() == mem.size());
            REQUIRE(sl.data() == mem.data());

            slice<uint8_t> subslice(mem, 10, 120);
            REQUIREABORTS(slice<uint8_t> subslice(mem, 10, 600));
            REQUIREABORTS(slice<uint8_t> subslice(mem, 0, 513));
            REQUIREABORTS(slice<uint8_t> subslice(mem, 100, 50));
            {
                slice<uint8_t> subslice_a(mem, 0, 512);
                slice<uint8_t> subslice_b(mem);
                REQUIRE(subslice_a == subslice_b);
            }
            REQUIRE(subslice.size() == 110);
            REQUIRE(subslice.data() == &mem[10]);

            static_assert(!std::is_default_constructible_v<slice<uint8_t>>,
                          "Slice should not be default constructible because "
                          "it is non-nullable");
            // not allowed
            // slice<uint8_t> slice_2;
        }

        SUBCASE("construct from single item")
        {
            int oneint[1] = {0};

            zl::slice<int> ints = zl::slice<int>::from_one((int&)oneint[0]);
            REQUIRE(ints.size() == 1);
            for (int i : ints)
                REQUIRE(i == oneint[0]);

            zl::slice<const int> ints_const =
                zl::slice<int>::from_one(oneint[0]);
            REQUIRE(ints.size() == 1);
            const int oneint_const[1] = {0};
            ints_const = zl::slice<int>::from_one(oneint[0]);
            REQUIRE(ints.size() == 1);
        }

        SUBCASE("const correctness")
        {
            int oneint[1] = {0};
            zl::slice<int> ints = zl::slice<int>::from_one(oneint[0]);
            static_assert(std::is_same_v<int*, decltype(ints.data())>);

            zl::slice<const int> ints_const =
                zl::slice<int>::from_one((int&)oneint[0]);
            static_assert(
                std::is_same_v<const int*, decltype(ints_const.data())>);

            auto get_nonconst_by_const_ref = [](const zl::slice<int>& guy) {
                static_assert(std::is_same_v<decltype(guy.data()), int*>);
                return guy;
            };

            auto copy = get_nonconst_by_const_ref(ints);
            static_assert(std::is_same_v<decltype(copy.data()), int*>);

            zl::slice<const int> cint_1 =
                zl::slice<const int>::from_one((const int&)oneint[0]);
            zl::slice<const int> cint_2(cint_1);
        }

        SUBCASE("empty subslice")
        {
            std::array<uint8_t, 512> mem;

            slice<uint8_t> slice(mem, 0, 0);
            REQUIRE(slice.size() == 0);

            size_t index = 0;
            for (auto byte : slice) {
                ++index;
            }
            REQUIRE(index == 0);
        }

        SUBCASE("iteration")
        {
            std::array<uint8_t, 128> mem;
            slice<uint8_t> slice(mem);

            std::fill(slice.begin(), slice.end(), 0);
            uint8_t index = 0;
            for (auto& byte : slice) {
                REQUIRE(byte == 0);
                byte = index;
                ++index;
            }

            // make sure that also changed mem
            index = 0;
            for (auto byte : mem) {
                REQUIRE(byte == index);
                ++index;
            }
        }

        SUBCASE("const iteration")
        {
            std::array<uint8_t, 128> mem;
            std::fill(mem.begin(), mem.end(), 0);
            const slice<uint8_t> slice(mem);

            uint8_t index = 0;
            for (const auto& byte : slice) {
                REQUIRE(byte == 0);
                mem[index] = index;
                ++index;
            }

            // make sure that also changed slice
            index = 0;
            for (const auto& byte : slice) {
                REQUIRE(byte == index);
                ++index;
            }
        }

        SUBCASE("subslice construction")
        {
            std::array<uint8_t, 128> mem;
            slice<uint8_t> sl(mem);
            slice<uint8_t> subslice(sl, 10, 127);

            REQUIRE(subslice.size() < sl.size());
        }

        SUBCASE("slice of C array")
        {
            uint8_t mem[128];
            auto slice = raw_slice(mem[0], sizeof(mem));

            for (auto [byte, index] : enumerate_mut(slice)) {
                byte = index;
            }

            for (size_t i = 0; i < sizeof(mem); ++i) {
                REQUIRE(mem[i] == i);
            }
        }
    }
}
