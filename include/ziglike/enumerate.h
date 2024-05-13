#pragma once
#include <iterator>
#include <type_traits>

namespace zl {
template <typename Iterable, typename IteratorType, bool owns, bool is_const>
class enumerator;

template <typename T> struct reference_enumeration
{
  private:
    T &m_item;
    const size_t m_index;

  public:
    inline constexpr reference_enumeration(T &item,
                                           size_t index) ZIGLIKE_NOEXCEPT
        : m_item(item),
          m_index(index)
    {
    }

    template <size_t I> auto &get() const &
    {
        if constexpr (I == 0)
            return m_item;
        else if constexpr (I == 1)
            return m_index;
    }

    template <typename, typename, bool, bool> friend class enumerator;
};

template <typename T> struct const_reference_enumeration
{
  private:
    const T &m_item;
    const size_t m_index;

  public:
    inline constexpr const_reference_enumeration(const T &item,
                                                 size_t index) ZIGLIKE_NOEXCEPT
        : m_item(item),
          m_index(index)
    {
    }

    template <size_t I> auto const &get() const &
    {
        if constexpr (I == 0)
            return m_item;
        else if constexpr (I == 1)
            return m_index;
    }

    template <typename, typename, bool, bool> friend class enumerator;
};

template <typename T> struct value_enumeration
{
    static_assert(!std::is_reference_v<T>);
    static_assert(sizeof(T) <= sizeof(T &));

  private:
    const T m_item;
    const size_t m_index;

  public:
    inline constexpr value_enumeration(const T &item,
                                       size_t index) ZIGLIKE_NOEXCEPT
        : m_item(item),
          m_index(index)
    {
    }

    template <size_t I> auto get() const &
    {
        if constexpr (I == 0)
            return m_item;
        else if constexpr (I == 1)
            return m_index;
    }

    template <typename, typename, bool, bool> friend class enumerator;
};

} // namespace zl

// NOTE: modifying namespace std is well defined for std::tuple_size and
// std::tuple_element
template <class T>
// NOLINTNEXTLINE
struct std::tuple_size<zl::reference_enumeration<T>>
    : public std::integral_constant<size_t, 2>
{};
template <class T>
// NOLINTNEXTLINE
struct std::tuple_size<zl::const_reference_enumeration<T>>
    : public std::integral_constant<size_t, 2>
{};
template <class T>
// NOLINTNEXTLINE
struct std::tuple_size<zl::value_enumeration<T>>
    : public std::integral_constant<size_t, 2>
{};

// NOLINTNEXTLINE
template <class T> struct std::tuple_element<0, zl::reference_enumeration<T>>
{
    using type = T &;
};
// NOLINTNEXTLINE
template <class T> struct std::tuple_element<1, zl::reference_enumeration<T>>
{
    using type = const size_t;
};
template <class T>
// NOLINTNEXTLINE
struct std::tuple_element<0, zl::const_reference_enumeration<T>>
{
    using type = const T &;
};
template <class T>
// NOLINTNEXTLINE
struct std::tuple_element<1, zl::const_reference_enumeration<T>>
{
    using type = const size_t;
};

// NOLINTNEXTLINE
template <class T> struct std::tuple_element<0, zl::value_enumeration<T>>
{
    using type = const T;
};
// NOLINTNEXTLINE
template <class T> struct std::tuple_element<1, zl::value_enumeration<T>>
{
    using type = const size_t;
};

namespace zl {

template <typename Iterable, typename IteratorType, bool owns, bool is_const>
class enumerator
{
  private:
    static_assert(!std::is_reference_v<Iterable>);
    using reference_type =
        std::conditional_t<is_const, const Iterable &, Iterable &>;
    using internal_type = std::conditional_t<owns, Iterable, reference_type>;

    internal_type m_iterable;

  public:
    struct iterator;
    struct const_iterator;

    template <typename U = Iterable>
    inline constexpr enumerator(std::enable_if_t<owns, U &&> iterable)
        : m_iterable(std::forward<decltype(iterable)>(iterable))
    {
    }

    template <typename U = Iterable>
    inline constexpr enumerator(std::enable_if_t<is_const, const U &> iterable)
        : m_iterable(std::forward<decltype(iterable)>(iterable))
    {
    }

    template <typename U = Iterable>
    inline constexpr enumerator(std::enable_if_t<!owns, U &> iterable)
        : m_iterable(std::forward<decltype(iterable)>(iterable))
    {
    }

    inline constexpr const_iterator begin() const ZIGLIKE_NOEXCEPT
    {
        return const_iterator(m_iterable.begin(), 0);
    }

    inline constexpr const_iterator end() const ZIGLIKE_NOEXCEPT
    {
        return const_iterator(m_iterable.end(), m_iterable.size());
    }

    inline constexpr iterator begin() ZIGLIKE_NOEXCEPT
    {
        return iterator(m_iterable.begin(), 0);
    }

    inline constexpr iterator end() ZIGLIKE_NOEXCEPT
    {
        return iterator(m_iterable.end(), m_iterable.size());
    }

  private:
    template <typename T> struct dummy_pointer_iterator
    {
        static_assert(std::is_pointer_v<T>);
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_pointer_t<T>;
        using pointer = T;
        using reference = value_type &;
    };

  public:
    struct iterator
    {
        // either the parent types iterator or const iterator
        using parent_iterator_raw =
            std::conditional_t<is_const, typename Iterable::const_iterator,
                               typename Iterable::iterator>;
        // either parent_iterator_raw or a dummy for if its a pointer
        using parent_iterator =
            std::conditional_t<std::is_pointer_v<parent_iterator_raw>,
                               dummy_pointer_iterator<parent_iterator_raw>,
                               parent_iterator_raw>;
        // NOTE: always forward iterator regardless of iterable
        using iterator_category = std::forward_iterator_tag;
        using difference_type = typename parent_iterator::difference_type;
        using value_type = typename parent_iterator::value_type;
        using pointer = typename parent_iterator::pointer;
        using reference = typename parent_iterator::reference;

        inline constexpr iterator(const parent_iterator_raw &iter,
                                  size_t index) ZIGLIKE_NOEXCEPT
            : m_iter(iter),
              m_index(index)
        {
        }

        using reference_enumeration_type =
            std::conditional_t<is_const,
                               const_reference_enumeration<value_type>,
                               reference_enumeration<value_type>>;
        using enumeration_type =
            std::conditional_t<sizeof(value_type) <= sizeof(value_type *),
                               value_enumeration<value_type>,
                               reference_enumeration_type>;

        inline constexpr enumeration_type operator*() const ZIGLIKE_NOEXCEPT
        {
            return enumeration_type{
                *m_iter,
                size_t(m_index),
            };
        }

        // inline constexpr pointer operator->() ZIGLIKE_NOEXCEPT { return
        // m_iter.operator->(); }

        // Prefix increment
        inline constexpr iterator &operator++() ZIGLIKE_NOEXCEPT
        {
            ++m_iter;
            ++m_index;
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
            return a.m_iter == b.m_iter;
        };
        inline constexpr friend bool
        operator!=(const iterator &a, const iterator &b) ZIGLIKE_NOEXCEPT
        {
            return a.m_iter != b.m_iter;
        };

      private:
        size_t m_index;
        parent_iterator_raw m_iter;
    };
};

/// Returns the iterator with a good default for IteratorType
template <typename T, bool is_const = false> struct enumerator_for
{
    using underlying = std::remove_const_t<std::remove_reference_t<T>>;
    using type = enumerator<underlying,
                            typename std::remove_reference_t<T>::value_type &,
                            std::is_rvalue_reference_v<T>, is_const>;
};

template <typename T, bool is_const = false>
using enumerator_for_t = typename enumerator_for<T, is_const>::type;

template <typename T>
inline constexpr auto enumerate(T &&iterable)
    -> enumerator_for_t<decltype(iterable)>
{
    return enumerator_for_t<decltype(iterable)>(
        std::forward<decltype(iterable)>(iterable));
}

template <typename T>
inline constexpr auto enumerate(T &iterable)
    -> enumerator_for_t<decltype(iterable)>
{
    return enumerator_for_t<decltype(iterable)>(
        std::forward<decltype(iterable)>(iterable));
}

template <typename T>
inline constexpr auto enumerate(const T &iterable)
    -> enumerator_for_t<decltype(iterable), true>
{
    return enumerator_for_t<decltype(iterable), true>(
        std::forward<decltype(iterable)>(iterable));
}

/// enumerate variant for being explicitly const
template <typename T>
inline constexpr auto enumerate_const(const T &iterable)
    -> enumerator_for_t<decltype(iterable), true>
{
    return enumerator_for_t<decltype(iterable), true>(
        std::forward<decltype(iterable)>(iterable));
}

} // namespace zl
