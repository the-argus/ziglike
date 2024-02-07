#include "test_header.h"
// test header must be first
#include "ziglike/factory.h"
#include "ziglike/opt.h"

#include <vector>

using namespace zl;

TEST_SUITE("factory")
{
    TEST_CASE("make_back")
    {
        SUBCASE("vector with trivial type")
        {
            struct factoryable_t
            {
              private:
                int i;
                float j;

                inline constexpr factoryable_t(int _i, float _j) : i(_i), j(_j)
                {
                }

              public:
                static inline constexpr factoryable_t make(int i, float j)
                {
                    return factoryable_t(i, j);
                }

                static inline constexpr factoryable_t make()
                {
                    return factoryable_t(0, 0.0f);
                }

                [[nodiscard]] inline constexpr int get_i() const { return i; }
                [[nodiscard]] inline constexpr float get_j() const { return j; }
            };

            std::vector<factoryable_t> vec;
            make_back(vec, 1, 0.4f);
            REQUIRE(vec.size() == 1);
            REQUIRE(vec[0].get_i() == 1);
            REQUIRE(vec[0].get_j() == 0.4f);
            make_back(vec);
            REQUIRE(vec.size() == 2);
            REQUIRE(vec[1].get_i() == 0);
            REQUIRE(vec[1].get_j() == 0.0f);
        }
    }
}
