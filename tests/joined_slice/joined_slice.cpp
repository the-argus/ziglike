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
            zl::joined_slice<const int> nonconst_joined(nonconst_slice_of_slices);

            zl::joined_slice<const int> const_version(nonconst_joined);
        }
    }
}
