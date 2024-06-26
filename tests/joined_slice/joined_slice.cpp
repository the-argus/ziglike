#include "test_header.h"
// test header must be first
#include "ziglike/joined_slice.h"
#include <array>

static_assert(!std::is_default_constructible_v<zl::joined_slice<int>>);
static_assert(
    std::is_constructible_v<zl::joined_slice<int>, zl::slice<zl::slice<int>>>);
static_assert(std::is_constructible_v<zl::joined_slice<int>,
                                      zl::slice<const zl::slice<int>>>);
static_assert(std::is_constructible_v<zl::joined_slice<const int>,
                                      zl::slice<zl::slice<int>>>);
static_assert(std::is_constructible_v<zl::joined_slice<const int>,
                                      zl::slice<const zl::slice<int>>>);
static_assert(std::is_constructible_v<zl::joined_slice<const int>,
                                      zl::joined_slice<int>>);
static_assert(
    std::is_constructible_v<zl::joined_slice<int>, zl::joined_slice<int>>);
static_assert(!std::is_constructible_v<zl::joined_slice<int>,
                                       zl::joined_slice<const int>>);
static_assert(!std::is_constructible_v<zl::joined_slice<int>,
                                       zl::slice<zl::slice<const int>>>);

TEST_SUITE("joined_slice")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construct from slice of slices")
        {
            std::array<int, 10> mem;
            std::array<zl::slice<int>, 4> slices = {mem, mem, mem, mem};

            zl::slice<zl::slice<int>> slice_of_slices = slices;

            // BUG: this constructor doesn't work when this is const int instead
            // of int, same for std::array
            zl::joined_slice<int> joined(slices);
            zl::joined_slice<int> joined_2(slice_of_slices);
        }

        SUBCASE("const correctness during construction")
        {
            std::array<int, 10> mem;
            std::array<zl::slice<const int>, 4> slices = {mem, mem, mem, mem};
            zl::slice<const zl::slice<const int>> slice_of_slices = slices;
            zl::slice<zl::slice<const int>> nonconst_slice_of_slices = slices;

            zl::joined_slice<const int> const_joined(slice_of_slices);
            zl::joined_slice<const int> nonconst_joined(
                nonconst_slice_of_slices);

            zl::joined_slice<const int> const_version(nonconst_joined);
        }
    }

    TEST_CASE("functionality")
    {
        SUBCASE("iterate over nonconst")
        {
            std::array<int, 10> mem{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::array<zl::slice<int>, 4> slices = {mem, mem, mem, mem};

            zl::joined_slice<int> ints(slices);

            REQUIRE(ints.begin() != ints.end());

            int expected = 0;
            size_t count = 0;
            for (int i : ints) {
                REQUIRE(i == expected);
                ++expected;
                expected %= mem.size();
                ++count;
            }
            REQUIRE(count == mem.size() * slices.size());
        }
    }
}
