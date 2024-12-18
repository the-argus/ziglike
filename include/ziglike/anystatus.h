#pragma once
#include "res.h"
#include "status.h"
#include <cstdint>
#include <type_traits>

#ifndef ZIGLIKE_NOEXCEPT
#define ZIGLIKE_NOEXCEPT noexcept
#endif

namespace zl {
struct anystatus
{
  private:
    uint8_t m_status;

  public:
    [[nodiscard]] inline constexpr bool okay() const ZIGLIKE_NOEXCEPT
    {
        return m_status == 0;
    }

    [[nodiscard]] inline constexpr uint8_t err() const ZIGLIKE_NOEXCEPT
    {
        return m_status;
    }

    /// Can be constructed from a result, discarding the contents of the result
    /// and basically just storing the byte error code.
    template <typename T, typename Code>
    inline constexpr anystatus(const res<T, Code>& result) ZIGLIKE_NOEXCEPT
    {
        m_status = uint8_t(result.err());
    }

    /// Can be constructed from a status, removing the type information of the
    /// status.
    template <typename Code>
    inline constexpr anystatus(status<Code> status) ZIGLIKE_NOEXCEPT
    {
        m_status = uint8_t(status.err());
    }

    /// Can be constructed from a raw result errcode enum, which effectively
    /// just casts the enum value to a uint8_t.
    template <typename Code>
    inline constexpr anystatus(
        typename std::enable_if_t<
            std::is_enum_v<Code> && sizeof(Code) == 1 &&
                std::underlying_type_t<Code>(Code::Okay) == 0 &&
                (std::underlying_type_t<Code>(Code::ResultReleased) !=
                 std::underlying_type_t<Code>(Code::Okay)),
            Code>
            status) ZIGLIKE_NOEXCEPT
    {
        m_status = uint8_t(status);
    }

    /// Can be implicitly constructed from a bool, true being okay and false
    /// being not
    inline constexpr anystatus(bool status) ZIGLIKE_NOEXCEPT
        : m_status(status ? 0 : 255)
    {
    }
};
} // namespace zl
