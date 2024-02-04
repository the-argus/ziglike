#include "test_header.h"
// test header must be first
#include "testing_types.h"
#include "ziglike/res.h"

#include <type_traits>
#include <vector>

using namespace zl;

TEST_SUITE("res")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("Return status codes and success from functions")
        {
            auto getresiftrue = [](bool cond) -> res<trivial_t, StatusCodeA> {
                if (cond) {
                    return trivial_t{
                        .whatever = 10,
                        .nothing = nullptr,
                    };
                } else {
                    return StatusCodeA::BadAccess;
                }
            };

            REQUIRE((getresiftrue(true).okay()));
            REQUIRE((getresiftrue(true).release().whatever == 10));
            REQUIRE((!getresiftrue(false).okay()));
            REQUIRE(getresiftrue(false).err() == StatusCodeA::BadAccess);
        }

        SUBCASE("construct type directly into result")
        {
            struct constructed_type
            {
                const char *string = nullptr;
                inline constructed_type(const std::string &instr)
                {
                    string = instr.c_str();
                }
            };
            using result = res<constructed_type, StatusCodeA>;
            auto constructed_result = [](bool cond) -> result {
                return cond ? result(std::string("hello"))
                            : result(StatusCodeA::OOMIGuess);
            };

            REQUIRE(constructed_result(true).okay());
            REQUIRE(constructed_result(true).release().string != nullptr);
            REQUIRE(!constructed_result(false).okay());
            REQUIRE(constructed_result(false).err() == StatusCodeA::OOMIGuess);
        }
    }

    TEST_CASE("Functionality")
    {
#ifdef ZIGLIKE_USE_FMT
        SUBCASE("formattable")
        {
            using result = res<int, StatusCodeB>;
            using refresult = res<int &, StatusCodeB>;
            result result(10);
            int target = 10;
            refresult refresult(target);
            fmt::println("Result int: {}", result);
            fmt::println("Reference result int: {}", refresult);
            auto unused = result.release();
            auto unusedref = refresult.release();
            fmt::println("Released result: {}", result);
            fmt::println("Reference result int: {}", refresult);
        }
#endif

        SUBCASE("aborts on bad access")
        {
            using res = res<int, StatusCodeB>;
            res result(StatusCodeB::Nothing);
            REQUIREABORTS(result.release());
        }

        SUBCASE("Result released code after release is called")
        {
            using res = res<trivial_t, StatusCodeB>;
            res result(trivial_t{
                .whatever = 19,
                .nothing = nullptr,
            });
            REQUIRE(result.okay());
            REQUIRE(result.release().whatever == 19);
            REQUIRE(!result.okay());
            REQUIRE(result.err() == StatusCodeB::ResultReleased);
        }

        SUBCASE("std::vector result")
        {
            enum class VectorCreationStatusCode : uint8_t
            {
                Okay,
                ResultReleased,
                OOM,
            };

            using res = res<std::vector<size_t>, VectorCreationStatusCode>;

            res vec_result((std::vector<size_t>()));
            REQUIRE(vec_result.okay());
            auto vec = vec_result.release();
            REQUIRE(!vec_result.okay());
            vec.push_back(42);
            // pass a copy of the vector into the result
            res vec_result_modified(std::move(vec));

            // moving the result works fine
            auto passthrough = [](res &&result) -> res && {
                REQUIRE(result.okay());
                return std::move(result);
            };

            vec_result_modified = passthrough(std::move(vec_result_modified));

            REQUIRE(vec_result_modified.okay());
            std::vector<size_t> vec_modified = vec_result_modified.release();
            REQUIRE(vec_modified.size() == 1);
            REQUIRE(vec_modified[0] == 42);
        }

        SUBCASE("reference result")
        {
            enum class ReferenceCreationStatusCode : uint8_t
            {
                Okay,
                ResultReleased,
                NullReference,
            };

            using res = res<std::vector<int> &, ReferenceCreationStatusCode>;

            auto makeveciftrue = [](bool cond) -> res {
                if (cond) {
                    auto *vec = new std::vector<int>;
                    vec->push_back(5);
                    return *vec;
                } else {
                    return ReferenceCreationStatusCode::NullReference;
                }
            };

            REQUIRE(!makeveciftrue(false).okay());
            REQUIREABORTS(makeveciftrue(false).release());

            res result = makeveciftrue(true);
            REQUIRE(result.okay());
            auto &vec = result.release();
            REQUIRE(vec[0] == 5);
            vec.push_back(10);
            REQUIRE(!result.okay());
            delete &vec;
        }

        SUBCASE("const reference result")
        {
            enum class ReferenceCreationStatusCode : uint8_t
            {
                Okay,
                ResultReleased,
                NullReference,
            };

            using res =
                res<const std::vector<int> &, ReferenceCreationStatusCode>;

            auto makeveciftrue = [](bool cond) -> res {
                if (cond) {
                    auto *vec = new std::vector<int>;
                    return *vec;
                } else {
                    return ReferenceCreationStatusCode::NullReference;
                }
            };

            REQUIRE(!makeveciftrue(false).okay());
            REQUIREABORTS(makeveciftrue(false).release());

            res result = makeveciftrue(true);
            REQUIRE(result.okay());
            // this doesnt work
            // std::vector<int>& vec = result.release();
            const auto &vec = result.release();
            // this doesnt work
            // vec.push_back(10);
            REQUIRE(!result.okay());
            delete &vec;
        }

        SUBCASE("how much result copies its contents")
        {
            static int copies = 0;
            static int moves = 0;
            struct increment_on_copy_or_move
            {
                int one = 1;
                float two = 2;
                // NOLINTNEXTLINE
                constexpr increment_on_copy_or_move(int _one,
                                                    float _two) noexcept
                    : one(_one), two(_two)
                {
                }
                inline increment_on_copy_or_move &
                operator=(const increment_on_copy_or_move &other)
                {
                    if (&other == this)
                        return *this;
                    ++copies;
                    one = other.one;
                    two = other.two;
                    return *this;
                }
                inline increment_on_copy_or_move &
                operator=(increment_on_copy_or_move &&other) noexcept
                {
                    if (&other == this)
                        return *this;
                    ++moves;
                    one = other.one;
                    two = other.two;
                    other.one = 0;
                    other.two = 0;
                    return *this;
                }
                inline increment_on_copy_or_move(
                    const increment_on_copy_or_move &other)
                {
                    *this = other;
                }
                inline increment_on_copy_or_move(
                    increment_on_copy_or_move &&other) noexcept
                {
                    *this = std::move(other);
                }
            };

            enum class dummy_status_code_e : uint8_t
            {
                Okay,
                ResultReleased,
                DummyError,
            };

            using res = res<increment_on_copy_or_move, dummy_status_code_e>;

            // no copy, only move
            res res_1 = increment_on_copy_or_move(1, 2);
            REQUIRE(res_1.okay());
            REQUIRE(copies == 0);
            REQUIRE(moves == 1);
            auto &resref_1 = res_1.release_ref();
            REQUIRE(copies == 0);
            REQUIRE(moves == 1);
            res res_2 = increment_on_copy_or_move(1, 2);
            // moves increment to move the item into the result
            REQUIRE(moves == 2);
            REQUIRE(copies == 0);
            REQUIRE(res_2.okay());
            REQUIREABORTS(res_1.release()); // NOLINT
            increment_on_copy_or_move dummy_2 = res_2.release();
            // moves incremented because release() moves the item out of the
            // result
            REQUIRE(copies == 0);
            REQUIRE(moves == 3);
            increment_on_copy_or_move dummy_3 = dummy_2; // NOLINT
            REQUIRE(copies == 1);
            REQUIRE(moves == 3);
        }
    }
}
