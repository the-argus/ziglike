#pragma once
#include <type_traits>

namespace zl::detail {
// template specializations to detect whether a type is an instance of
template <class, template <typename> typename>
struct is_instance : public std::false_type
{
};

template <class Y, template <typename> typename U>
struct is_instance<U<Y>, U> : public std::true_type
{
};
} // namespace zl::detail
