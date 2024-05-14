#include "test_header.h"
// test header must be first
#include "ziglike/anystatus.h"
#include "ziglike/status.h"

#include <array>

using namespace zl;

enum class GenericError : uint8_t
{
    Okay,
    ResultReleased,
    Evil,
};
enum class OtherError : uint8_t
{
    Okay,
    ResultReleased,
    OOM,
    NotAllowed,
};

static_assert(sizeof(status<GenericError>) == 1,
              "status is not a single byte in size");

TEST_SUITE("status")
{
    TEST_CASE("Construction and type behavior")
    {
        SUBCASE("construction")
        {
            status<GenericError> stat(GenericError::Okay);
            status<GenericError> stat2 = GenericError::Okay;
            status<GenericError> stat3 = GenericError::Evil;
            REQUIRE(stat.okay());
            REQUIRE(stat2.okay());
            REQUIRE(!stat3.okay());
            std::array<decltype(stat), 3> statuses = {stat, stat2, stat3};
            for (auto s : statuses) {
                anystatus any = s;
                REQUIRE(s.okay() == any.okay());
            }
        }
        SUBCASE("copy assignment")
        {
            status<GenericError> stat(GenericError::Okay);
            auto stat2 = stat;

            static_assert(std::is_copy_assignable_v<decltype(stat)>,
                          "Status is not copy assignable");

            REQUIRE((stat2.okay() && stat.okay()));
        }
    }
    TEST_CASE("functionality")
    {
        //  NOTE: this case is really just to make sure it compiles
        SUBCASE("turning different statuses into anystatus")
        {
            auto memalloc = []() -> status<OtherError> {
                void* bytes = malloc(100);
                if (!bytes)
                    return OtherError::OOM;
                free(bytes);
                return OtherError::Okay;
            };

            auto floatmath = []() -> status<GenericError> {
                const long i = 12378389479823989;
                const long j = 85743323894782374;

                const float test_one =
                    static_cast<float>(i) / static_cast<float>(j);
                const double test_two =
                    static_cast<double>(i) / static_cast<double>(j);

                if (std::abs(static_cast<double>(test_one) - test_two) <
                    double(0.1)) {
                    return GenericError::Okay;
                }
                return GenericError::Evil;
            };

            auto dostuff = [memalloc, floatmath]() -> anystatus {
                auto stat = memalloc();
                if (!stat.okay())
                    return stat;
                auto stat2 = floatmath();
                return stat2;
            };

            dostuff();
        }

        SUBCASE("anystatus conversion at runtime")
        {
            auto fakealloc = [](bool should_alloc) -> status<OtherError> {
                if (!should_alloc)
                    return OtherError::NotAllowed;
                return OtherError::Okay;
            };

            auto yesorno = [](bool cond) -> status<GenericError> {
                if (cond)
                    return GenericError::Okay;
                else
                    return GenericError::Evil;
            };

            auto dostuff = [fakealloc, yesorno](bool one,
                                                bool two) -> anystatus {
                auto status1 = fakealloc(one);
                if (!status1.okay())
                    return status1;
                return yesorno(two);
            };

            REQUIRE(dostuff(true, true).okay());
            REQUIRE(!dostuff(false, true).okay());
            REQUIRE(!dostuff(true, false).okay());
            REQUIRE(!dostuff(false, false).okay());
        }
    }
}
