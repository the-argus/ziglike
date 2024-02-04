#pragma once

#include "detail/abort.h"
#include <cstdint>
#include <functional>
#include <utility>

#ifdef ZIGLIKE_USE_FMT
#include <fmt/core.h>
#endif

#ifndef ZIGLIKE_NOEXCEPT
#define ZIGLIKE_NOEXCEPT noexcept
#endif

#ifndef ZIGLIKE_NO_SMALL_OPTIONAL_SLICE
#include "./detail/isinstance.h"
#include "./slice.h"
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

#ifndef ZIGLIKE_OPTIONAL_ALLOW_POINTERS
    static_assert(!std::is_pointer_v<T>,
                  "Attempt to create an optional pointer. Pointers are already "
                  "optional. Maybe make an optional reference instead?");
#endif

    static constexpr bool is_reference = std::is_lvalue_reference<T>::value;

#ifndef ZIGLIKE_NO_SMALL_OPTIONAL_SLICE
    static constexpr bool is_slice = zl::detail::is_instance<T, zl::slice>{};
#endif

  private:
    union raw_optional
    {
        T some;
        uint8_t none;
        inline ~raw_optional() ZIGLIKE_NOEXCEPT {}
    };

    struct members
    {
        bool has_value = false;
        raw_optional value{.none = 0};
    };

    struct members_ref
    {
        std::remove_reference_t<T> *pointer = nullptr;
    };

    using members_t =
        typename std::conditional<is_reference, members_ref, members>::type;

    members_t m;

  public:
    /// Returns true if its safe to call value(), false otherwise.
    [[nodiscard]] inline bool has_value() const ZIGLIKE_NOEXCEPT
    {
        if constexpr (is_reference) {
            return m.pointer != nullptr;
        } else {
            return m.has_value;
        }
    }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    [[nodiscard]] inline typename std::conditional<is_reference, T, T &>::type
    value() ZIGLIKE_NOEXCEPT
    {
        if (!has_value()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        if constexpr (is_reference) {
            return *m.pointer;
        } else {
            return std::ref(m.value.some);
        }
    }

    inline typename std::conditional<is_reference, const T, const T &>::type
    value_const() const ZIGLIKE_NOEXCEPT
    {
        if (!has_value()) [[unlikely]] {
            ZIGLIKE_ABORT();
        }
        if constexpr (is_reference) {
            return *m.pointer;
        } else {
            return std::ref(m.value.some);
        }
    }

    /// Extract the inner value of the optional, or abort the program. Check
    /// has_value() before calling this.
    inline typename std::conditional<is_reference, const T, const T &>::type
    value() const ZIGLIKE_NOEXCEPT
    {
        return value_const();
    }

    /// Call destructor of internal type, or just reset it if it doesnt have one
    inline void reset() ZIGLIKE_NOEXCEPT
    {
        if (!has_value()) [[unlikely]] {
            return;
        }

        if constexpr (!is_reference) {
            m.value.some.~T();
            m.has_value = false;
        } else {
            m.pointer = nullptr;
        }
    }

    /// Non-reference types can be constructed directly in to the optional
    template <typename... Args>
    inline void emplace(Args &&...args) ZIGLIKE_NOEXCEPT
    {
        static_assert(
            !is_reference,
            "emplace() cannot be called on reference-type optionals.");
        if (!is_reference) {
            if (has_value()) [[unlikely]] {
                reset();
            }
            static_assert(std::is_constructible_v<T, decltype(args)...>,
                          "Type T is not constructible with given arguments");
            new (&m.value.some) T(std::forward<decltype(args)>(args)...);
            m.has_value = true;
        }
    }

    inline constexpr opt() ZIGLIKE_NOEXCEPT {}
    inline ~opt() ZIGLIKE_NOEXCEPT
    {
        if constexpr (!is_reference) {
            reset();
            m.has_value = false;
        } else {
            m.pointer = nullptr;
        }
    }

    /// Able to assign a moved type if the type is moveable
    template <typename MaybeT = T>
    inline constexpr opt &
    operator=(typename std::enable_if_t<
              !is_reference && std::is_constructible_v<MaybeT, MaybeT &&>,
              MaybeT> &&something) ZIGLIKE_NOEXCEPT
    {
        if (m.has_value) {
            m.value.some.~MaybeT();
        }
        new (&m.value.some) MaybeT(std::move(something));
        m.has_value = true;
        return *this;
    }

    template <typename MaybeT = T>
    inline constexpr opt(
        typename std::enable_if_t<
            !is_reference && std::is_constructible_v<MaybeT, MaybeT &&>, MaybeT>
            &&something) ZIGLIKE_NOEXCEPT
    {
        new (&m.value.some) MaybeT(std::move(something));
        m.has_value = true;
    }

    /// Trivially copyable types can also be assigned into their optionals
    template <typename MaybeT = T>
    inline constexpr opt &
    operator=(const typename std::enable_if_t<
              !is_reference &&
                  std::is_trivially_constructible_v<MaybeT, const MaybeT &>,
              MaybeT> &something) ZIGLIKE_NOEXCEPT
    {
        if (m.has_value) {
            m.value.some.~MaybeT();
        }
        new (&m.value.some) MaybeT(something);
        m.has_value = true;
        return *this;
    }

    // copy constructor
    template <typename MaybeT = T>
    inline constexpr opt(
        const typename std::enable_if_t<
            !is_reference &&
                std::is_trivially_constructible_v<MaybeT, const MaybeT &>,
            MaybeT> &something) ZIGLIKE_NOEXCEPT
    {
        new (&m.value.some) T(something);
        m.has_value = true;
    }

    /// Optional containing a reference type can be directly constructed from
    /// the reference type
    template <typename MaybeT = T>
    inline constexpr opt(typename std::enable_if_t<is_reference, MaybeT>
                             something) ZIGLIKE_NOEXCEPT
    {
        m.pointer = std::addressof(something);
    }

    /// Reference types can be assigned to an optional to overwrite it.
    template <typename MaybeT = T>
    inline constexpr opt &
    operator=(typename std::enable_if_t<is_reference, MaybeT> something)
        ZIGLIKE_NOEXCEPT
    {
        m.pointer = &something;
        return *this;
    }

    /// NOTE: References are not able to use the == overload because
    /// it would not be clear whether it was a strict comparison or not. (ie is
    /// it comparing the address or the contents of the thing at the address?)

    /// EQUALS: Compare an optional to another optional of the same type
    template <typename ThisType>
    inline constexpr friend bool operator==(
        const typename std::enable_if_t<
            !is_reference && std::is_same_v<ThisType, opt>, ThisType> &self,
        const ThisType &other) ZIGLIKE_NOEXCEPT
    {
        if (!self.has_value()) {
            return !other.has_value();
        } else {
            return !other.has_value() ? false
                                      : self.m.value.some == other.m.value.some;
        }
    }

    /// EQUALS: Compare an optional to something of its contained type
    template <typename MaybeT = T>
    inline constexpr bool operator==(
        const std::enable_if_t<!is_reference && std::is_same_v<MaybeT, T>,
                               MaybeT> &other) ZIGLIKE_NOEXCEPT
    {
        return !has_value() ? false : m.value.some == other;
    }

    /// NOT EQUALS: Compare an optional to another optional of the same type
    template <typename ThisType>
    inline constexpr friend bool operator!=(
        const typename std::enable_if_t<
            !is_reference && std::is_same_v<ThisType, opt>, ThisType> &self,
        const ThisType &other) ZIGLIKE_NOEXCEPT
    {
        if (!self.has_value()) {
            return other.has_value();
        } else {
            return !other.has_value() ? true
                                      : self.m.value.some != other.m.value.some;
        }
    }

    /// NOT EQUALS: Compare an optional to something of its contained type
    template <typename MaybeT = T>
    inline constexpr bool operator!=(
        const std::enable_if_t<!is_reference, MaybeT> &other) ZIGLIKE_NOEXCEPT
    {
        return !has_value() ? true : m.value.some != other;
    }

    // Strict comparison for an optional reference: return true if the optional
    // reference is pointing at the item passed in.
    /// ONLY VALID FOR REFERENCE-TYPE OPTIONALS
    template <typename OtherType = T>
    inline constexpr bool strict_compare(
        const std::enable_if_t<is_reference && (std::is_same_v<OtherType, T> ||
                                                std::is_same_v<OtherType, opt>),
                               OtherType> &other) ZIGLIKE_NOEXCEPT
    {
        if constexpr (std::is_same_v<OtherType, T>) {
            if (!has_value())
                return false;
            return std::addressof(other) == m.pointer;
        } else {
            return other.m.pointer == m.pointer;
        }
    }

    /// Loose comparison: compare the thing the optional reference is pointing
    /// to to the item passed in. They do not have to literally be the same
    /// object.
    /// ONLY VALID FOR REFERENCE-TYPE OPTIONALS
    template <typename OtherType = T>
    inline constexpr bool loose_compare(
        const std::enable_if_t<is_reference && (std::is_same_v<OtherType, T> ||
                                                std::is_same_v<OtherType, opt>),
                               OtherType> &other) ZIGLIKE_NOEXCEPT
    {
        if constexpr (std::is_same_v<OtherType, T>) {
            if (!has_value())
                return false;
            return other == *m.pointer;
        } else {
            return *other.m.pointer == *m.pointer;
        }
    }

#ifdef ZIGLIKE_USE_FMT
    friend struct fmt::formatter<opt>;
#endif
};
} // namespace zl

#ifdef ZIGLIKE_USE_FMT
template <typename T> struct fmt::formatter<zl::opt<T>>
{
    static_assert(
        fmt::is_formattable<T>::value,
        "Attempt to format an optional whose contents is not formattable.");

    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    format_context::iterator format(const zl::opt<T> &optional,
                                    format_context &ctx) const
    {
        if (optional.has_value()) {
            if constexpr (zl::opt<T>::is_reference) {
                return fmt::format_to(ctx.out(), "{}", *optional.m.pointer);
            } else {
                return fmt::format_to(ctx.out(), "{}", optional.m.value.some);
            }
        }
        return fmt::format_to(ctx.out(), "null");
    }
};
#endif
