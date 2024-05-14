#include "test_header.h"
// test header must be first
#include "testing_types.h"
#include "ziglike/enumerate.h"
#include "ziglike/opt.h"
#include "ziglike/slice.h"

using namespace zl;

#ifndef ZIGLIKE_NO_SMALL_OPTIONAL_SLICE
static_assert(sizeof(opt<slice<int>>) == sizeof(slice<int>),
              "Optional slice types are a different size than slices");
#endif

static_assert(sizeof(opt<int&>) == sizeof(int*),
              "Optional reference types are a different size than pointers");

TEST_SUITE("opt")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("Default construction")
        {
            opt<int> def;
            REQUIRE(!def.has_value());
            REQUIRE(def != 0);
        }

        SUBCASE("Construction with value")
        {
            opt<int> has = 10;
            REQUIRE(has.has_value());
            REQUIRE(has == 10);
            REQUIRE(has.value() == 10);
        }

        SUBCASE("comparison")
        {
            opt<int> one = 100;
            opt<int> two;
            REQUIRE(one != two);
            REQUIRE(two != one);
            two = 200;
            {
                bool both_has_value = one.has_value() && two.has_value();
                REQUIRE(both_has_value);
            }
            REQUIRE(one != two);
            REQUIRE(two != one);
            one.reset();
            two.reset();
            {
                bool both_dont_has_value = !one.has_value() && !two.has_value();
                REQUIRE(both_dont_has_value);
            }
            REQUIRE(one == two);
            one = 1;
            two = 1;
            REQUIRE(one == two);
        }

        SUBCASE("convertible to bool")
        {
            opt<int> nothing;
            REQUIRE(!nothing);
            REQUIRE(!nothing.has_value());
            opt<int> something = 1;
            REQUIRE(something);
            REQUIRE(something.has_value());

            auto bool_to_optional = [](bool input) -> opt<int> {
                return input ? opt<int>(3478) : opt<int>{};
            };

            if (auto result = bool_to_optional(true)) {
                REQUIRE(result);
                REQUIRE(result.value() == 3478);
                REQUIRE(result == 3478);

                REQUIRE(result != opt<int>(3477));
                REQUIRE(result != 3477);
                REQUIRE(result != opt<int>{});
            }

            if (auto result = bool_to_optional(false)) {
                REQUIRE(false); // should never happen
            }
        }
    }

    TEST_CASE("Functionality")
    {
        SUBCASE("Resetting")
        {
            opt<std::vector<int>> vec;
            // null by default
            REQUIRE(!vec.has_value());
            vec.emplace();
            REQUIRE(vec.has_value());
            vec.value().push_back(42);
            REQUIRE(vec.value()[0] == 42);
            vec.reset();
            REQUIRE(!vec.has_value());
        }

        SUBCASE("Aborts on null")
        {
            opt<int> nope;
            REQUIREABORTS(++nope.value());
        }

        SUBCASE("moving non-trivially-copyable type")
        {
            moveable_t moveguy;
            int bytes = std::snprintf(moveguy.nothing, 50, "nope");

            opt<moveable_t> maybe_moveguy = std::move(moveguy);
            REQUIRE(maybe_moveguy.has_value());
            // and this shouldnt work
            // opt<moveable_t> maybe_moveguy = moveguy;

            REQUIRE(strcmp(maybe_moveguy.value().nothing, "nope") == 0);
        }

        SUBCASE("optional reference types")
        {
            int test = 10;
            opt<int&> testref;
            opt<int&> testref2;
            REQUIRE(!testref.has_value());
            REQUIRE(!testref2.has_value());
            testref = test;
            REQUIRE(testref.value() == test);
            REQUIRE(testref.strict_compare(test));
            REQUIRE(testref.loose_compare(test));

            // difference between loose and strict comparison
            int test2 = 10;
            REQUIRE(testref.loose_compare(test2));
            REQUIRE(!testref.strict_compare(test2));
        }

        SUBCASE("inplace return")
        {
            static size_t copy_count = 0;
            struct BigThing
            {
                std::array<int, 300> numbers;
                inline constexpr BigThing() noexcept : numbers({}) {}
                inline constexpr BigThing(const BigThing& other) noexcept
                    : numbers(other.numbers)
                {
                    ++copy_count;
                }
                BigThing& operator=(const BigThing& other) = delete;
            };

            auto try_make_big_thing = [](bool should_succeed) -> opt<BigThing> {
                if (should_succeed)
                    return opt<BigThing>{std::in_place};
                else
                    return {};
            };

            opt<BigThing> maybe_thing = try_make_big_thing(true);
            opt<BigThing> maybe_not_thing = try_make_big_thing(true);
            REQUIRE(copy_count == 0);
            // one copy required to get it out of the optional
            BigThing thing = try_make_big_thing(true).value();
            REQUIRE(copy_count == 1);
        }

        SUBCASE("emplace")
        {
            opt<std::vector<int>> mvec;
            REQUIRE(!mvec.has_value());
            mvec.emplace();
            REQUIRE(mvec.has_value());
        }

        SUBCASE("safely return copies from value optionals")
        {
            const auto get_maybe_int = []() -> opt<int> { return 1; };

            static_assert(
                std::is_same_v<decltype(get_maybe_int().value()), int&&>);

            int my_int = get_maybe_int().value();
            my_int++;
            if (my_int == 2)
                printf("no asan!\n");
        }

        SUBCASE("safely return copies from slice optionals")
        {
            std::array<uint8_t, 512> mem;
            const auto get_maybe_slice = [&mem]() -> opt<slice<uint8_t>> {
                return slice<uint8_t>(mem);
            };

            static_assert(std::is_same_v<decltype(get_maybe_slice().value()),
                                         slice<uint8_t>&&>);

            slice<uint8_t> my_slice = get_maybe_slice().value();
            std::fill(my_slice.begin(), my_slice.end(), 0);
            for (auto byte : mem) {
                REQUIRE(byte == 0);
            }
        }

#ifndef ZIGLIKE_NO_SMALL_OPTIONAL_SLICE
        SUBCASE("emplace slice types")
        {
            std::array<uint8_t, 128> bytes{0};
            for (auto [byte, index] : enumerate_mut(bytes)) {
                byte = index;
            }

            opt<slice<uint8_t>> maybe_bytes;
            maybe_bytes.emplace(bytes);

            for (auto [byte, index] : enumerate(maybe_bytes.value())) {
                REQUIRE(byte == bytes[index]);
            }
        }
#endif

        SUBCASE("moving or copying trivially copyable type")
        {
            struct thing
            {
                int yeah = 10234;
                bool no = false;
            };

            thing copyguy;
            opt<thing> maybe_copyguy = copyguy;
            // identical to:
            opt<thing> maybe_copyguy_moved = std::move(copyguy); // NOLINT

            REQUIRE(maybe_copyguy.has_value());
            REQUIRE(maybe_copyguy_moved.has_value());
        }

        SUBCASE("copying slice")
        {
            std::array<uint8_t, 128> bytes;
            opt<slice<uint8_t>> maybe_bytes(bytes);

            opt<slice<uint8_t>> other_maybe_bytes(maybe_bytes);
            REQUIRE(other_maybe_bytes == maybe_bytes);

            slice<uint8_t> bytes_slice = other_maybe_bytes.value();
        }

#ifdef ZIGLIKE_USE_FMT
        SUBCASE("formattable")
        {
            opt<std::string_view> str;
            str = "yello";
            fmt::println("optional string BEFORE: {}", str);
            str.reset();
            fmt::println("optional string AFTER: {}", str);

            std::string_view target = "reference yello";
            opt<std::string_view&> refstr(target);
            fmt::println("optional reference string BEFORE: {}", refstr);
            refstr.reset();
            fmt::println("optional reference string AFTER: {}", refstr);

            std::array<uint8_t, 128> bytes;
            opt<slice<uint8_t>> maybe_bytes;
            maybe_bytes.emplace(bytes);
            fmt::println("optional slice: {}", maybe_bytes);
        }
#endif
    }
}
