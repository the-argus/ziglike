#pragma once
#include "ziglike/opt.h"
#include <type_traits>
#include <utility>

namespace zl {

template <
    typename Callable,
    // template only works if Callable is in fact callable with no
    // arguments, and returns void
    typename std::enable_if<std::is_same_v<void, decltype(Callable())>>::type>
class defer
{
    opt<Callable &&> statement;

  public:
    explicit defer(Callable &&f) : statement(std::forward<Callable>(f)) {}

    // you cannot move or copy or really mess with a defer at all
    defer &operator=(const defer &) = delete;
    defer &operator=(defer &&) = delete;
    defer(const defer &) = delete;
    defer(defer &&) = delete;

    inline constexpr void dont() { statement.reset(); }

    ~defer()
    {
        if (statement)
            statement();
    }
};
} // namespace zl
