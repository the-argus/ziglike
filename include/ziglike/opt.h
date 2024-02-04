#pragma once

#include "detail/abort.h"
#include <cstdint>
#include <functional>
#include <utility>

#ifdef ZIGLIKE_USE_FMT
#include "formatters/opt.h"
namespace fmt {
template <typename T> struct formatter;
}
#endif

namespace zl {
/// Optional (nullable) type. Accepts basic types, pointers, structs, etc, or
/// lvalue references.
template <typename T> class opt
{
  public:
    // type constraints
    static_assert((!std::is_reference<T>::value && std::is_destructible_v<T>) ||
                      (std::is_reference<T>::value &&
                       std::is_lvalue_reference_v<T>),
                  "Optional type must be either nothrow destructible or an "
                  "lvalue reference type.");

    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

  private:
    struct wrapper
    {
        T item;
        wrapper() = delete;
        inline constexpr wrapper(T item) : item(item){};
    };

    union raw_optional
    {
        typename std::conditional<is_reference, wrapper, T>::type some;
        uint8_t none;
        inline ~raw_optional() noexcept {}
    };
    bool m_has_value = false;
    raw_optional m_value{.none = 0};

  public:
    /// Returns true if its safe to call value(), false otherwise.
    [[nodiscard]] inline bool has_value() const noexcept { return m_has_value; }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    [[nodiscard]] inline typename std::conditional<is_reference, T, T &>::type
    value() noexcept
    {
        if (!m_has_value) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::ref(m_value.some);
        }
    }

    inline typename std::conditional<is_reference, const T, const T &>::type
    value_const() const noexcept
    {
        if (!m_has_value) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::ref(m_value.some);
        }
    }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    inline typename std::conditional<is_reference, const T, const T &>::type
    value() const noexcept
    {
        if (!m_has_value) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        if constexpr (is_reference) {
            return m_value.some.item;
        } else {
            return std::ref(m_value.some);
        }
    }

    /// Non reference types can have their destructors explicitly called
    inline void reset() noexcept
    {
        if (!has_value()) [[unlikely]] {
            return;
        }
        if constexpr (!is_reference) {
            m_value.some.~T();
        }
        m_has_value = false;
    }

    /// Non-reference types can be constructed directly in to the optional
    template <typename... Args> inline void emplace(Args &&...args) noexcept
    {
        static_assert(
            !is_reference,
            "emplace() cannot be called on reference-type optionals.");
        if (has_value()) [[unlikely]] {
            reset();
        }
        static_assert(std::is_constructible_v<T, decltype(args)...>,
                      "Type T is not constructible with given arguments");
        new (&m_value.some) T(std::forward<decltype(args)>(args)...);
        m_has_value = true;
    }

    /// Contextually convertible to bool
    inline constexpr operator bool() const noexcept { return m_has_value; }

    inline constexpr opt() noexcept {}
    inline ~opt() noexcept
    {
        if constexpr (!is_reference) {
            reset();
        }
        m_has_value = false;
    }

    /// Able to assign a moved type if the type is moveable
    template <typename MaybeT = T>
    inline constexpr opt &
    operator=(typename std::enable_if_t<
              !is_reference && std::is_constructible_v<MaybeT, MaybeT &&>,
              MaybeT> &&something) noexcept
    {
        if (m_has_value) {
            m_value.some.~MaybeT();
        }
        new (&m_value.some) MaybeT(std::move(something));
        m_has_value = true;
        return *this;
    }

    template <typename MaybeT = T>
    inline constexpr opt(
        typename std::enable_if_t<
            !is_reference && std::is_constructible_v<MaybeT, MaybeT &&>, MaybeT>
            &&something) noexcept
    {
        new (&m_value.some) MaybeT(std::move(something));
        m_has_value = true;
    }

    /// Trivially copyable types can also be assigned into their optionals
    template <typename MaybeT = T>
    inline constexpr opt &
    operator=(const typename std::enable_if_t<
              !is_reference &&
                  std::is_trivially_constructible_v<MaybeT, const MaybeT &>,
              MaybeT> &something) noexcept
    {
        if (m_has_value) {
            m_value.some.~MaybeT();
        }
        new (&m_value.some) MaybeT(something);
        m_has_value = true;
        return *this;
    }

    template <typename MaybeT = T>
    inline constexpr opt(
        const typename std::enable_if_t<
            !is_reference &&
                std::is_trivially_constructible_v<MaybeT, const MaybeT &>,
            MaybeT> &something) noexcept
    {
        new (&m_value.some) T(something);
        m_has_value = true;
    }

    /// Optional containing a reference type can be directly constructed from
    /// the reference type
    template <typename MaybeT = T>
    inline constexpr opt(
        typename std::enable_if_t<is_reference, MaybeT> something) noexcept
    {
        new (&m_value.some) wrapper(something);
        m_has_value = true;
    }

    /// Reference types can be assigned to an optional to overwrite it.
    template <typename MaybeT = T>
    inline constexpr opt &operator=(
        typename std::enable_if_t<is_reference, MaybeT> something) noexcept
    {
        // no need to destroy the potentially existing reference since
        // references dont have destructors
        new (&m_value.some) wrapper(something);
        m_has_value = true;
        return *this;
    }

    /// Comparable to non-optional versions of the same type, only if not a
    /// reference. NOTE: References are not able to use the == overload because
    /// it would not be clear whether it was a strict comparison or not. (ie is
    /// it comparing the address or the contents of the thing at the address?)
    template <typename MaybeT = T>
    inline constexpr bool
    operator==(const typename std::enable_if_t<!is_reference, MaybeT>
                   &something) noexcept
    {
        if (!has_value())
            return false;
        else {
            return m_value.some == something;
        }
    }

#ifdef ZIGLIKE_USE_FMT
    friend struct fmt::formatter<opt>;
#endif
};
} // namespace zl
