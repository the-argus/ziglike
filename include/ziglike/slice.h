#pragma once
#include <cassert>
// we want std iterator compatibility
#ifndef ZIGLIKE_SLICE_NO_ITERATOR
#include <iterator>
#endif

#include "ziglike/detail/abort.h"

#ifdef ZIGLIKE_USE_FMT
#include <fmt/core.h>
#endif

namespace zl {

// forward decls
template <typename T> class slice;
template <typename T>
[[nodiscard]] constexpr inline slice<T> raw_slice(T &data,
                                                  size_t size) noexcept;

/// A non-owning reference to a section of a contiguously allocated array of
/// type T. Intended to be passed around like a pointer.
template <typename T> class slice
{
  private:
    size_t m_elements;
    T *m_data;

    inline constexpr slice(T *data, size_t size) noexcept
    {
        assert(data != nullptr);
        m_data = data;
        m_elements = size;
    }

  public:
    using type = T;

#ifndef ZIGLIKE_SLICE_NO_ITERATOR
    struct iterator;
    struct const_iterator;

    // make an iterable container
    inline constexpr iterator begin() noexcept { return iterator(m_data); }
    inline constexpr iterator end() noexcept
    {
        return iterator(m_data + m_elements);
    }

    inline constexpr const_iterator begin() const noexcept
    {
        return const_iterator(m_data);
    }
    inline constexpr const_iterator end() const noexcept
    {
        return const_iterator(m_data + m_elements);
    }
#endif

    // raw access to contents
    [[nodiscard]] inline constexpr T *data() noexcept { return m_data; }
    [[nodiscard]] inline constexpr const T *data() const noexcept
    {
        return m_data;
    }
    [[nodiscard]] inline constexpr size_t size() const noexcept
    {
        return m_elements;
    }

    /// Wrap a contiguous stdlib container which has data() and size() functions
    template <typename Container>
    inline constexpr slice<T>(Container &container) noexcept
        : slice(container.data(), container.size())
    {
    }

    template <>
    inline constexpr slice(const slice &other) noexcept
        : slice(other.data(), other.size())
    {
    }

    /// Take a slice of contiguous stdlib container from one index to another
    /// (from is less than to)
    template <typename Container>
    inline constexpr slice<T>(Container &container, size_t from,
                              size_t to) noexcept
    {
        if (from > to || to > container.size()) [[unlikely]]
            ZIGLIKE_ABORT();
        m_elements = to - from;
        m_data = &container.data()[from];
    }

    inline constexpr friend bool operator==(const slice &a,
                                            const slice &b) noexcept
    {
        return a.m_elements == b.m_elements && a.m_data == b.m_data;
    };

    inline constexpr friend bool operator!=(const slice &a,
                                            const slice &b) noexcept
    {
        return a.m_elements != b.m_elements || a.m_data != b.m_data;
    };

    /// Can't default construct a slice since its always a reference to another
    /// thing.
    slice() = delete;

#ifndef ZIGLIKE_SLICE_NO_ITERATOR
    /// The slice's iterator is the majority of the class. It is only a forwards
    /// iterator for simplicities sake (I really never iterate in any other way)
    struct iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type *;
        using reference = value_type &;

        inline constexpr pointer ptr() noexcept { return m_ptr; }

        inline constexpr iterator(pointer ptr) noexcept : m_ptr(ptr) {}

        inline constexpr reference operator*() const noexcept { return *m_ptr; }

        inline constexpr pointer operator->() noexcept { return m_ptr; }

        // Prefix increment
        inline constexpr iterator &operator++() noexcept
        {
            ++m_ptr;
            return *this;
        }

        // Postfix increment
        // NOLINTNEXTLINE
        inline constexpr iterator operator++(int) noexcept
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        inline constexpr friend bool operator==(const iterator &a,
                                                const iterator &b) noexcept
        {
            return a.m_ptr == b.m_ptr;
        };
        inline constexpr friend bool operator!=(const iterator &a,
                                                const iterator &b) noexcept
        {
            return a.m_ptr != b.m_ptr;
        };

            // TODO: implement this (ran into template deducible type problems)
#ifdef ZIGLIKE_USE_FMT
        friend struct fmt::formatter<iterator>;
#endif

      private:
        pointer m_ptr;
    };

    struct const_iterator
    {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = const value_type *;
        using reference = const value_type &;

        inline constexpr pointer ptr() noexcept { return m_ptr; }

        inline constexpr const_iterator(pointer ptr) noexcept : m_ptr(ptr) {}

        inline constexpr reference operator*() const noexcept { return *m_ptr; }

        inline constexpr pointer operator->() noexcept { return m_ptr; }

        // Prefix increment
        inline constexpr const_iterator &operator++() noexcept
        {
            ++m_ptr;
            return *this;
        }

        // Postfix increment
        // NOLINTNEXTLINE
        inline constexpr const_iterator operator++(int) noexcept
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        inline constexpr friend bool
        operator==(const const_iterator &a, const const_iterator &b) noexcept
        {
            return a.m_ptr == b.m_ptr;
        };
        inline constexpr friend bool
        operator!=(const const_iterator &a, const const_iterator &b) noexcept
        {
            return a.m_ptr != b.m_ptr;
        };

            // TODO: implement this (ran into template deducible type problems)
#ifdef ZIGLIKE_USE_FMT
        friend struct fmt::formatter<const_iterator>;
#endif

      private:
        pointer m_ptr;
    };
#endif

    friend constexpr inline slice zl::raw_slice(T &data, size_t size) noexcept;
#ifdef ZIGLIKE_USE_FMT
    // friend struct fmt::formatter<slice>;
#endif
};

/// Construct a slice point to a buffer of memory. Requires that data is not
/// nullptr. Aborts the program if data is nullptr.
template <typename T>
[[nodiscard]] constexpr inline slice<T> raw_slice(T &data, size_t size) noexcept
{
    return slice<T>(std::addressof(data), size);
}

} // namespace zl

#ifdef ZIGLIKE_USE_FMT
template <typename T> struct fmt::formatter<zl::slice<T>>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    auto format(const zl::slice<T> &slice, format_context &ctx) const
        -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "[{:p} -> {}]",
                              reinterpret_cast<void *>(slice.m_data),
                              slice.m_elements);
    }
};
#endif
