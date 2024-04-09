#pragma once

// the whole point of this type is to provide seamless iteration over a bunch of
// slices, it doesnt make sense to use it with no iterator
#ifndef ZIGLIKE_SLICE_NO_ITERATOR

#ifdef ZIGLIKE_USE_FMT
#include <fmt/core.h>
#endif

#include "ziglike/detail/abort.h"
#include "ziglike/detail/isinstance.h"
#include "ziglike/slice.h"

namespace zl {
template <typename T> class joined_slice
{
  private:
    slice<const slice<T>> m_slices;

    using TNonConst = typename std::conditional_t<std::is_const_v<T>,
                                                  std::remove_const_t<T>, T>;
    using TConst = typename std::conditional_t<std::is_const_v<T>, T, const T>;

    struct uninstantiable
    {
        friend struct joined_slice;

      private:
        uninstantiable() = default;
    };

    enum class Iterpoint : uint8_t
    {
        Begin,
        End,
    };

  public:
    using type = T;

#ifndef ZIGLIKE_SLICE_NO_ITERATOR
    struct iterator;
    struct const_iterator;

    using correct_iterator =
        std::conditional_t<std::is_const_v<T>, const_iterator, iterator>;

    inline constexpr correct_iterator begin() const ZIGLIKE_NOEXCEPT
    {
        return correct_iterator(m_slices, Iterpoint::Begin);
    }
    inline constexpr correct_iterator end() const ZIGLIKE_NOEXCEPT
    {
        return correct_iterator(m_slices, Iterpoint::End);
    }
#endif

    // Non-templated constructor allowing conversion from a const slice of
    // slices to a joined slice. A conversion from a non-const slice of slices
    // is not provided because it would cause ambiguity. This constructor only
    // exists to allow for std::array and std::vector to be implicitly converted
    // into a joined slice
    inline constexpr joined_slice(slice<const slice<TNonConst>> slices)
        ZIGLIKE_NOEXCEPT : m_slices(slices)
    {
    }

    template <typename U>
    inline constexpr joined_slice(
        U slices,
        std::enable_if_t<std::is_same_v<slice<const slice<TNonConst>>, U> ||
                             std::is_same_v<slice<slice<TNonConst>>, U>,
                         uninstantiable> = {}) ZIGLIKE_NOEXCEPT
        : m_slices(slices)
    {
    }

    template <typename U>
    inline constexpr joined_slice(
        U slices,
        std::enable_if_t<
            (std::is_same_v<U, slice<const slice<TConst>>> ||
             std::is_same_v<U, slice<slice<TConst>>>)&&std::is_const_v<T>,
            uninstantiable> = {}) ZIGLIKE_NOEXCEPT : m_slices(slices)
    {
    }

    /// A joined slice can always be constructed from a nonconst variant of
    /// itself
    inline constexpr joined_slice(joined_slice<TNonConst> other)
        ZIGLIKE_NOEXCEPT : m_slices(other.m_slices)
    {
    }

    template <typename U>
    inline constexpr joined_slice(
        U other, std::enable_if_t<std::is_same_v<U, joined_slice<TConst>> &&
                                      std::is_const_v<T>,
                                  uninstantiable> = {}) ZIGLIKE_NOEXCEPT
    {
    }

    joined_slice &operator=(const joined_slice &) = default;
    joined_slice(const joined_slice &) = default;

    inline constexpr friend bool
    operator==(const joined_slice &a, const joined_slice &b) ZIGLIKE_NOEXCEPT
    {
        return a.m_slices == b.m_slices;
    };

    inline constexpr friend bool
    operator!=(const joined_slice &a, const joined_slice &b) ZIGLIKE_NOEXCEPT
    {
        return a.m_slices != b.m_slices;
    };
};
} // namespace zl
#endif
