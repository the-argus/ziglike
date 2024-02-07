#pragma once
#include "ziglike/detail/isinstance.h"
#include "ziglike/opt.h"
#include <type_traits>
#include <utility>

namespace zl {
// disabled via partial template specialization
template <typename Callable> class defer
{
    opt<Callable &&> statement;
    static_assert(std::is_invocable_r<void, Callable>::value,
                  "Callable is not invocable with no arguments, and/or it does "
                  "not return void.");
    static_assert(!(detail::is_instance<Callable, std::function>{}),
                  "Do not try to defer a std::function!");

  public:
    explicit defer(Callable &&f) : statement(std::forward<Callable>(f)) {}

    // you cannot move or copy or really mess with a defer at all
    defer &operator=(const defer &) = delete;
    defer &operator=(defer &&) = delete;
    defer(const defer &) = delete;
    defer(defer &&) = delete;

    inline constexpr void cancel() { statement.reset(); }

    ~defer()
    {
        if (statement.has_value())
            statement.value()();
    }
};
} // namespace zl
