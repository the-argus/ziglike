#pragma once

#include "detail/abort.h"
#include <cstdint>
#include <type_traits>

#ifdef ZIGLIKE_USE_FMT
#include "formatters/res.h"
namespace fmt {
template <typename T> struct formatter;
}
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
        inline constexpr wrapper(T item) noexcept : item(item){};
    };

    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

    union raw_optional
    {
        typename std::conditional<is_reference, wrapper, T>::type some;
        uint8_t none;
        ~raw_optional() noexcept {}
    };

    // keeps track of the status. if it is StatusCode::Okay, then we have
    // something inside the raw_optional. Otherwise, its contents are undefined.
    StatusCode m_status;
    raw_optional m_value{.none = 0};

  public:
    using type = T;
    using err_type = StatusCode;

    /// Returns true if it is safe to call release(), otherwise false.
    [[nodiscard]] inline constexpr bool okay() const noexcept
    {
        return m_status == StatusCode::Okay;
    }

    [[nodiscard]] inline constexpr StatusCode err() const noexcept
    {
        return m_status;
    }

    /// Return a copy of the internal contents of the result. If this result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function.
    [[nodiscard]] inline typename std::conditional<is_reference, T, T &&>::type
    release() noexcept
    {
        if (!okay()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m_status = StatusCode::ResultReleased;
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::move(m_value.some);
        }
    }

    /// Return a reference to the data inside the result. This reference
    /// becomes invalid when the result is destroyed or moved. If the result is
    /// an error, this aborts the program. Check okay() before calling this
    /// function. Do not try to call release() or release_ref() again, after
    /// calling release() or release_ref() once, the result is invalidated.
    template <typename MaybeT = T>
    [[nodiscard]] inline typename std::enable_if_t<!is_reference, MaybeT> &
    release_ref() &noexcept
    {
        if (!okay()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m_status = StatusCode::ResultReleased;
        return m_value.some;
    }

    /// Cannot call release_ref on an rvalue result
    // T &release_ref() && noexcept requires(!is_reference) = delete;

    /// Can construct a T directly into the result, if the result doesn't
    /// contain a reference type.
    template <typename ThisType = res, typename... Args>
    static inline constexpr std::enable_if_t<
        !is_reference && std::is_constructible_v<T, Args...> &&
            std::is_same_v<ThisType, res>,
        ThisType> &&
    construct_into(Args &&...args) noexcept
    {
        res empty;
        new (&empty.m_value.some) T(std::forward<decltype(args)>(args)...);
        assert(empty.okay());
        return empty;
    }

    /// if T is a reference type, then you can construct a result from it
    template <typename MaybeT = T>
    inline constexpr res(
        typename std::enable_if_t<is_reference, MaybeT> success) noexcept
    {
        m_status = StatusCode::Okay;
        new (&m_value.some) wrapper(success);
    }

    /// Wrapped type can moved into a result
    template <typename MaybeT = T>
    inline constexpr res(
        typename std::enable_if_t<!is_reference, MaybeT> &&success) noexcept
    {
        static_assert(std::is_move_constructible_v<T>,
                      "Attempt to move a type T into a result but it's not "
                      "move constructible.");
        m_status = StatusCode::Okay;
        new (&m_value.some) T(std::move(success));
    }

    /// A statuscode can also be implicitly converted to a result
    inline constexpr res(StatusCode failure) noexcept
    {
        if (failure == StatusCode::Okay) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        m_status = failure;
    }

    /// Copy constructor only available if the wrapped type is trivially
    /// copy constructible.
    template <typename ThisType = res>
    inline constexpr res(
        const typename std::enable_if_t<(is_reference ||
                                         std::is_trivially_copy_constructible_v<
                                             T>)&&std::is_same_v<ThisType, res>,
                                        ThisType> &other) noexcept
    {
        if (&other == this) [[unlikely]]
            return *this;
        if (other.okay()) {
            if constexpr (is_reference) {
                m_value.some = other.m_value.some.item;
            } else {
                m_value.some = other.m_value.some;
            }
        }
        m_status = other.m_status;
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
            ThisType> &&other) noexcept
    {
        if (&other == this) [[unlikely]]
            return *this;
        else if (other.okay()) {
            if constexpr (is_reference) {
                m_value.some.item = other.m_value.some.item;
            } else {
                m_value.some = std::move(other.m_value.some);
            }
        }
        m_status = other.m_status;
        // make it an error to access a result after it has been moved into
        // another
        other.m_status = StatusCode::ResultReleased;
        return *this;
    }

    inline ~res() noexcept
    {
        if constexpr (!is_reference) {
            // always call the destructor of the thing if it was emplaced
            if (okay()) {
                m_value.some.~T();
            }
        }
        m_status = StatusCode::ResultReleased;
    }

#ifdef ZIGLIKE_USE_FMT
    friend struct fmt::formatter<res>;
#endif

  private:
    inline constexpr explicit res() noexcept { m_status = StatusCode::Okay; }
};
} // namespace zl
