#pragma once
#include <tuple>
#include <utility>

namespace zl {

// taken directly from
// https://quuxplusone.github.io/blog/2018/05/17/super-elider-round-2/

template <class F> class with_result_of_t
{
    F&& function;

  public:
    using T = decltype(std::declval<F&&>()());
    explicit with_result_of_t(F&& f) : function(std::forward<F>(f)) {}
    operator T() { return function(); }
};

template <class F> inline with_result_of_t<F> with_result_of(F&& f)
{
    return with_result_of_t<F>(std::forward<F>(f));
}

/// T::make an item into a container which has "emplace_back".
template <typename Container, typename... Args>
inline void make_back(Container& container, Args&&... args)
{
    container.emplace_back(with_result_of(
        [args =
             std::make_tuple(std::forward<Args>(args)...)]() mutable -> auto {
            return std::apply(
                [](auto&&... args) {
                    return Container::value_type::make(
                        std::forward<decltype(args)>(args)...);
                },
                std::move(args));
        }));
}
} // namespace zl
