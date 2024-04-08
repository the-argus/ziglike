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
static_assert(!std::is_constructible_v<zl::joined_slice<int>,
                                       zl::joined_slice<const int>>);
static_assert(!std::is_constructible_v<zl::joined_slice<int>,
                                       zl::slice<zl::slice<const int>>>);

// single constructors
static_assert(std::is_constructible_v<zl::joined_slice<int>, zl::slice<int>>);
static_assert(
    !std::is_constructible_v<zl::joined_slice<int>, zl::slice<const int>>);
static_assert(
    std::is_constructible_v<zl::joined_slice<const int>, zl::slice<const int>>);
static_assert(
    std::is_constructible_v<zl::joined_slice<const int>, zl::slice<int>>);

static_assert(
    std::is_constructible_v<zl::joined_slice<int>, std::array<int, 5>>);
static_assert(
    std::is_constructible_v<zl::joined_slice<const int>, std::array<int, 5>>);

TEST_SUITE("joined_slice")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construction") {}
    }
}
