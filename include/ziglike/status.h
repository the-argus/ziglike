#pragma once
#include <type_traits>

namespace zl {
/// Wrapper around a errcode to give it a similar interface to a result_t.
template <typename errcode_e> struct status
{
  public:
    static_assert(
        std::is_enum_v<errcode_e> && sizeof(errcode_e) == 1 &&
            typename std::underlying_type<errcode_e>::type(errcode_e::Okay) ==
                0 &&
            (typename std::underlying_type<errcode_e>::type(
                 errcode_e::ResultReleased) !=
             typename std::underlying_type<errcode_e>::type(errcode_e::Okay)),
        "Bad enum errorcode type provided to status_t. Make sure it is only a "
        "byte in size, and that the Okay entry is = 0.");

  private:
    errcode_e m_status;

  public:
    [[nodiscard]] inline constexpr bool okay() const noexcept
    {
        return m_status == errcode_e::Okay;
    }
    [[nodiscard]] inline constexpr errcode_e err() const noexcept
    {
        return m_status;
    }
    inline constexpr status(errcode_e failure) noexcept { m_status = failure; }
};
} // namespace zl
