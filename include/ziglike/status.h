#pragma once
#include <type_traits>

namespace zl {
/// Wrapper around a errcode to give it a similar interface to a result_t.
template <typename Errcode> struct status
{
  public:
    static_assert(
        std::is_enum_v<Errcode> && sizeof(Errcode) == 1 &&
            typename std::underlying_type<Errcode>::type(Errcode::Okay) == 0 &&
            (typename std::underlying_type<Errcode>::type(
                 Errcode::ResultReleased) !=
             typename std::underlying_type<Errcode>::type(Errcode::Okay)),
        "Bad enum errorcode type provided to status_t. Make sure it is only a "
        "byte in size, and that the Okay entry is = 0.");

  private:
    Errcode m_status;

  public:
    [[nodiscard]] inline constexpr bool okay() const noexcept
    {
        return m_status == Errcode::Okay;
    }
    [[nodiscard]] inline constexpr Errcode err() const noexcept
    {
        return m_status;
    }
    inline constexpr status(Errcode failure) noexcept { m_status = failure; }
};
} // namespace zl
