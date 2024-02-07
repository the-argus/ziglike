#pragma once
// #include "ziglike/detail/isinstance.h"
// #include <functional>
#include <type_traits>

namespace zl {

/// An object which takes a function and then calls that function when it is
/// destroyed.
template <typename Callable> class defer
{
    Callable *statement;
    static_assert(std::is_invocable_r<void, Callable>::value,
                  "Callable is not invocable with no arguments, and/or it does "
                  "not return void.");
    // static_assert(!(detail::is_instance<Callable, std::function>{}),
    //               "Do not try to defer a std::function!");

  public:
    // NOTE: takes the address of the callable, meaning passing anything other
    // than a lambda (like a std::function) is a bad idea
    explicit defer(Callable &&f) : statement(&f) {}

    // you cannot move or copy or really mess with a defer at all
    defer &operator=(const defer &) = delete;
    defer &operator=(defer &&) = delete;
    defer(const defer &) = delete;
    defer(defer &&) = delete;

    inline constexpr void cancel() { statement = nullptr; }

    ~defer()
    {
        if (statement)
            (*statement)();
    }
};
} // namespace zl
