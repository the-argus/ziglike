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

    struct iterator;

    inline constexpr iterator begin() const ZIGLIKE_NOEXCEPT
    {
        return iterator(m_slices, Iterpoint::Begin);
    }
    inline constexpr iterator end() const ZIGLIKE_NOEXCEPT
    {
        return iterator(m_slices, Iterpoint::End);
    }

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

    struct iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type *;
        using reference = value_type &;

        inline constexpr pointer ptr() const ZIGLIKE_NOEXCEPT
        {
            assert(m_slice_index < m_slices.size());
            assert(m_item_index < m_slices.data()[m_slice_index].size());
            return std::addressof(
                m_slices.data()[m_slice_index].data()[m_item_index]);
        }

        inline constexpr iterator(slice<const slice<T>> slices,
                                  Iterpoint iterpoint) ZIGLIKE_NOEXCEPT
            : m_slices(slices)
        {
            switch (iterpoint) {
            case Iterpoint::Begin:
                m_slice_index = 0;
                m_item_index = 0;
                break;
            case Iterpoint::End:
                m_slice_index = slices.size();
                m_item_index = 0;
                break;
            }
        }

        inline constexpr reference operator*() const ZIGLIKE_NOEXCEPT
        {
            return *ptr();
        }

        inline constexpr pointer operator->() const ZIGLIKE_NOEXCEPT
        {
            return ptr();
        }

        // Prefix increment
        inline constexpr iterator &operator++() ZIGLIKE_NOEXCEPT
        {
            assert(m_slices.size() != 0);
            if (m_slices.size() == 0) [[unlikely]]
                std::abort();
            ++m_item_index;
            if (m_slices.data()[m_slice_index].size() <= m_item_index) {
                ++m_slice_index;
                m_item_index = 0;
            }
            return *this;
        }

        // Postfix increment
        // NOLINTNEXTLINE
        inline constexpr iterator operator++(int) ZIGLIKE_NOEXCEPT
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        inline constexpr friend bool
        operator==(const iterator &a, const iterator &b) ZIGLIKE_NOEXCEPT
        {
            return a.m_slices == b.m_slices &&
                   a.m_slice_index == b.m_slice_index &&
                   a.m_item_index == b.m_item_index;
        };

        inline constexpr friend bool
        operator!=(const iterator &a, const iterator &b) ZIGLIKE_NOEXCEPT
        {
            return a.m_slices != b.m_slices ||
                   a.m_slice_index != b.m_slice_index ||
                   a.m_item_index != b.m_item_index;
        };

      private:
        size_t m_slice_index;
        size_t m_item_index;
        slice<const slice<T>> m_slices;
    };
};
} // namespace zl
#endif
