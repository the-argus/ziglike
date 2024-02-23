#pragma once

#include "detail/abort.h"
#include <cstdint>
#include <type_traits>
#include <utility> // std::in_place_t

#ifdef ZIGLIKE_USE_FMT
#include <fmt/core.h>
#endif

#ifndef ZIGLIKE_NOEXCEPT
#define ZIGLIKE_NOEXCEPT noexcept
#endif

namespace zl {
/// A result which is either a type T or a status code about why failure
/// occurred. StatusCode must be an 8-bit enum with an entry called "Okay"
/// equal to 0, and another entry called ResultReleased.
template <typename T, typename StatusCode> class res
{
  public:
    static_assert(
        std::is_lvalue_reference<T>::value ||
            (std::is_destructible_v<T> &&
             // type must be either moveable or trivially copyable, otherwise it
             // cant be returned from/moved out of a function
             (std::is_move_constructible_v<T> ||
              std::is_trivially_copy_constructible_v<T>)),
        "Invalid type passed to res's first template argument. The type "
        "must either be a lvalue reference, trivially copy constructible, or "
        "move constructible.");

    static_assert(
        std::is_enum_v<StatusCode> && sizeof(StatusCode) == 1 &&
            typename std::underlying_type<StatusCode>::type(StatusCode::Okay) ==
                0 &&
            (typename std::underlying_type<StatusCode>::type(
                 StatusCode::ResultReleased) !=
             typename std::underlying_type<StatusCode>::type(StatusCode::Okay)),
        "Bad enum errorcode type provided to res. Make sure it is only a "
        "byte in size, and that the Okay entry is = 0.");

  private:
    /// wrapper struct which just exits so that we can put reference types
    /// inside of the unione
    struct wrapper
    {
        T item;
        wrapper() = delete;
        inline constexpr wrapper(T item) ZIGLIKE_NOEXCEPT : item(item){};
    };

    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

    union raw_optional
    {
        typename std::conditional<is_reference, wrapper, T>::type some;
        uint8_t none;
        ~raw_optional() ZIGLIKE_NOEXCEPT {}
    };

    struct members
    {
        StatusCode status;
        raw_optional value{.none = 0};
    };

    members m;

  public:
    using type = T;
    using err_type = StatusCode;

    /// Returns true if it is safe to call release(), otherwise false.
    [[nodiscard]] inline constexpr bool okay() const ZIGLIKE_NOEXCEPT
    {
        return m.status == StatusCode::Okay;
    }

    [[nodiscard]] inline constexpr StatusCode err() const ZIGLIKE_NOEXCEPT
    {
        return m.status;
    }

    /// Return a copy of the internal contents of the result. If this result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function.
    [[nodiscard]] inline typename std::conditional<is_reference, T, T &&>::type
    release() ZIGLIKE_NOEXCEPT
    {
        if (!okay()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m.status = StatusCode::ResultReleased;
        if constexpr (is_reference) {
            return m.value.some.item;
        } else {
            return std::move(m.value.some);
        }
    }

    /// Return a reference to the data inside the result. This reference
    /// becomes invalid when the result is destroyed or moved. If the result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function. Do not try to call release() or release_ref() again, after
    /// calling release() or release_ref() once, the result is invalidated.
    template <typename MaybeT = T>
        [[nodiscard]] inline typename std::enable_if_t<!is_reference, MaybeT> &
        release_ref() &
        ZIGLIKE_NOEXCEPT
    {
        if (!okay()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m.status = StatusCode::ResultReleased;
        return m.value.some;
    }

    template <typename MaybeT = T, typename... Args>
    inline constexpr res(
        std::enable_if_t<!is_reference && std::is_constructible_v<T, Args...>,
                         std::in_place_t>,
        Args &&...args) noexcept
    {
        static_assert(std::is_nothrow_constructible_v<T, Args...>,
                      "Attempt to construct in place but constructor invoked "
                      "can throw exceptions.");
        m.status = StatusCode::Okay;
        new (&m.value.some) T(std::forward<Args>(args)...);
    }

    /// if T is a reference type, then you can construct a result from it
    template <typename MaybeT = T>
    inline constexpr res(typename std::enable_if_t<is_reference, MaybeT>
                             success) ZIGLIKE_NOEXCEPT
    {
        m.status = StatusCode::Okay;
        new (&m.value.some) wrapper(success);
    }

    /// Wrapped type can moved into a result
    template <typename MaybeT = T>
    inline constexpr res(
        typename std::enable_if_t<
            !is_reference && std::is_move_constructible_v<T>, MaybeT> &&success)
        ZIGLIKE_NOEXCEPT
    {
        static_assert(std::is_nothrow_move_constructible_v<T>,
                      "Attempt to use move constructor, but it throws and "
                      "function is marked noexcept.");
        m.status = StatusCode::Okay;
        new (&m.value.some) T(std::move(success));
    }

    /// A statuscode can also be implicitly converted to a result
    inline constexpr res(StatusCode failure) ZIGLIKE_NOEXCEPT
    {
        if (failure == StatusCode::Okay) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m.status = failure;
    }

    /// Copy constructor only available if the wrapped type is trivially
    /// copy constructible.
    template <typename ThisType = res>
    inline constexpr res(
        const typename std::enable_if_t<(is_reference ||
                                         std::is_trivially_copy_constructible_v<
                                             T>)&&std::is_same_v<ThisType, res>,
                                        ThisType> &other) ZIGLIKE_NOEXCEPT
    {
        if (&other == this) [[unlikely]]
            return *this;
        if (other.okay()) {
            if constexpr (is_reference) {
                m.value.some = other.m.value.some.item;
            } else {
                m.value.some = other.m.value.some;
            }
        }
        m.status = other.m.status;
        return *this;
    }

    // Result cannot be assigned to, only constructed and then released.
    res &operator=(const res &other) = delete;
    res &operator=(res &&other) = delete;

    /// Move construction of result, requires that T is a reference or that it's
    /// move constructible.
    template <typename ThisType = res>
    inline constexpr res(
        typename std::enable_if_t<
            (is_reference ||
             std::is_move_constructible_v<T>)&&std::is_same_v<ThisType, res>,
            ThisType> &&other) ZIGLIKE_NOEXCEPT
    {
        if (other.okay()) {
            if constexpr (is_reference) {
                m.value.some.item = other.m.value.some.item;
            } else {
                m.value.some = std::move(other.m.value.some);
            }
        }
        m.status = other.m.status;
        // make it an error to access a result after it has been moved into
        // another
        other.m.status = StatusCode::ResultReleased;
    }

    inline ~res() ZIGLIKE_NOEXCEPT
    {
        if constexpr (!is_reference) {
            if (okay()) {
                m.value.some.~T();
            }
        }
        m.status = StatusCode::ResultReleased;
    }

#ifdef ZIGLIKE_USE_FMT
    friend struct fmt::formatter<res>;
#endif

  private:
    inline constexpr explicit res() ZIGLIKE_NOEXCEPT
    {
        m.status = StatusCode::Okay;
    }
};
} // namespace zl

#ifdef ZIGLIKE_USE_FMT
template <typename T, typename StatusCode>
struct fmt::formatter<zl::res<T, StatusCode>>
{
    static_assert(
        fmt::is_formattable<T>::value,
        "Attempt to format a zl::res whose contents are not formattable.");
    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();
        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");
        return it;
    }

    format_context::iterator format(const zl::res<T, StatusCode> &result,
                                    format_context &ctx) const
    {
        if (result.okay()) {
            if constexpr (zl::res<T, StatusCode>::is_reference) {
                return fmt::format_to(ctx.out(), "{}",
                                      result.m.value.some.item);
            } else {
                return fmt::format_to(ctx.out(), "{}", result.m.value.some);
            }
        } else {
            if constexpr (fmt::is_formattable<
                              decltype(result.m.status)>::value) {
                return fmt::format_to(ctx.out(), "err {}", result.m.status);
            } else {
                return fmt::format_to(
                    ctx.out(), "err {}",
                    std::underlying_type_t<decltype(result.m.status)>(
                        result.m.status));
            }
        }
    }
};
#endif
