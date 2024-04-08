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

    inline constexpr joined_slice(slice<const slice<TNonConst>> slices) noexcept
        : m_slices(slices) ZIGLIKE_NOEXCEPT
    {
    }

    /// A joined slice can always be constructed from a nonconst variant of
    /// itself
    inline constexpr joined_slice(joined_slice<TNonConst> other)
        ZIGLIKE_NOEXCEPT : m_slices(other.m_slices)
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
