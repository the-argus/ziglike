#include "test_header.h"
// test header must be first
#include "ziglike/defer.h"
#include "ziglike/opt.h"
#include <array>
#include <unordered_set>

using namespace zl;

TEST_SUITE("defer")
{
    TEST_CASE("functionality")
    {
        SUBCASE("defer that does nothing")
        {
            defer([]() {});
        }

        SUBCASE("defer that adds to number")
        {
            int counter = 0;

            {
                defer set_to_zero([&counter]() { counter = 0; });
                for (size_t i = 0; i < 10; ++i) {
                    defer increment([&counter]() { counter++; });
                    static_assert(sizeof(increment) == sizeof(void*),
                                  "Defer object not the same size as pointer");
                    REQUIRE(counter == i);
                }
            }

            REQUIRE(counter == 0);
        }

        SUBCASE("conditionally cancel defer")
        {
            std::unordered_set<void*> malloced_stuff;
            auto fakemalloc = [&malloced_stuff](size_t bytes) -> void* {
                void* mem = malloc(bytes);
                REQUIRE(mem);
                malloced_stuff.insert(mem);
                return mem;
            };

            auto fakefree = [&malloced_stuff](void* mem) {
                malloced_stuff.erase(mem);
                free(mem);
            };

            // make sure fakemalloc and fakefree work
            REQUIRE(malloced_stuff.size() == 0);
            void* mem = fakemalloc(100);
            REQUIRE(malloced_stuff.size() == 1);
            fakefree(mem);
            REQUIRE(malloced_stuff.size() == 0);

            auto getmems = [fakemalloc, fakefree](
                               bool fail_halfway) -> opt<std::array<void*, 3>> {
                void* first_mem = fakemalloc(100);
                if (!first_mem)
                    return {};
                defer free_first_mem(
                    [first_mem, fakefree]() { fakefree(first_mem); });

                void* second_mem = fakemalloc(100);
                if (!second_mem)
                    return {};
                defer free_second_mem(
                    [second_mem, fakefree]() { fakefree(second_mem); });

                if (fail_halfway)
                    return {};

                void* third_mem = fakemalloc(100);
                if (!third_mem)
                    return {};

                // okay, all initialization is good, dont free anything
                free_first_mem.cancel();
                free_second_mem.cancel();

                return std::array<void*, 3>{first_mem, second_mem, third_mem};
            };

            auto maybe_mems = getmems(false);
            if (maybe_mems.has_value()) {
                REQUIRE(malloced_stuff.size() == 3);
                for (void* mem : maybe_mems.value()) {
                    fakefree(mem);
                }
            }
            REQUIRE(malloced_stuff.size() == 0);
            maybe_mems = getmems(true);
            // all stuff should be cleaned up if we fail halfway through init
            REQUIRE(malloced_stuff.size() == 0);
        }
    }
}
