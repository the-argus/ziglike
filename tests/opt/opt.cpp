#include "test_header.h"
// test header must be first
#include "testing_types.h"
#include "ziglike/opt.h"

using namespace zl;

#ifndef ZIGLIKE_NO_SMALL_OPTIONAL_SLICE
static_assert(sizeof(opt<int &>) == sizeof(int *),
              "Optional reference types are a different size than pointers");
#endif

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
            REQUIRE(!nothing.has_value());
            opt<int> something = 1;
            REQUIRE(something.has_value());
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
            opt<int &> testref;
            opt<int &> testref2;
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
            opt<thing> maybe_copyguy_moved = std::move(copyguy);

            REQUIRE(maybe_copyguy.has_value());
            REQUIRE(maybe_copyguy_moved.has_value());
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
            opt<std::string_view &> refstr(target);
            fmt::println("optional reference string BEFORE: {}", refstr);
            refstr.reset();
            fmt::println("optional reference string AFTER: {}", refstr);
        }
#endif
    }
}
