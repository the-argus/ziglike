#pragma once
#include <cstddef>
#include <type_traits>

namespace zl::detail {
template <typename T, typename Dummy = T> class is_container
{
  public:
    static constexpr bool value = false;
};

// stolen from https://retroscience.net/cpp-detect-functions-template.html
// tbh- I'm not sure if this always works. the ... args vs. single int
// specialization thing seems like a hack and I don't really understand it.
// TODO: figure out whats going on in the test function part of this template
// magic
template <typename T>
class is_container<
    T, typename std::enable_if_t<std::is_union_v<T> || std::is_class_v<T>, T>>
{
    /// test function which always returns false, unless the following
    /// specialization is matched
    template <typename> static std::false_type test(...);

    /// Function signature template specialization which takes exactly one int
    template <typename U>
    static auto test(int)
        -> decltype(std::declval<U>().data(), std::true_type());

    template <typename> static std::false_type size_test(...);

    template <typename U>
    static auto size_test(int)
        -> decltype(std::declval<U>().size(), std::true_type());

    static constexpr bool has_data =
        std::is_same<decltype(test<T>(0)), std::true_type>::value
        //&& std::is_pointer_v<decltype(std::declval<T>().data())>
        ;

    static constexpr bool has_size =
        std::is_same<decltype(size_test<T>(0)), std::true_type>::value
        //&& std::is_same_v<decltype(std::declval<T>().size()), size_t>
        ;

  public:
    static constexpr bool value = has_data && has_size;
};

template <typename T> constexpr bool is_container_v = is_container<T>::value;

} // namespace zl::detail
