# ARCHIVED

Ziglike is archived. This code has been greatly improved and is now found in
[okaylib](https://github.com/the-argus/okaylib). Okaylib is better tested, more
performant (opt and res types are actually constexpr and trivial whenever possible)
and provides far more features.

Some reimplementations of the Zig error union and optional types for C++ 17 and up.

Header-only.

Could also be considered a sort of C++17 backport of std::expected and std::span.

## Documentation

- [`zl::res`](./doc/res.md) : replaces exceptions with minimal overhead and no footguns, using error-code enums.
- [`zl::opt`](./doc/opt.md) : wraps a type and makes it nullable. Similar to std::optional, however it has slightly different semantics and supports reference types. An `opt<T&>` is the same size as a `T*`.
- [`zl::slice`](./doc/slice.md) : a struct which has a pointer to an array, and a `size_t` number of things. Very similar to `std::span`, but its non-nullable. Also, it works with C++17. An `opt<slice<T>>` is the same size as a `slice<T>`.
- A rudimentary recreation of Zig's `defer` statement.
- Utilities for replacing constructors with factory functions, namely the
  [Super-Constructing Super-Elider](https://quuxplusone.github.io/blog/2018/05/17/super-elider-round-2/).

## Examples

Here are some code examples of how code written with these types might look.

### `zl::res`

With `res`, you can add an additional byte on to your return values in order to
encode failure/success information. This means you have to write handling code
at the call site (unlike exceptions) but there is little overhead and the caller
always knows what they're getting into (unlike exceptions). Here is a very simple
allocation function being called, which is not using `res` to its full potential
but serves as a good example to show how it works:

```cpp
using namespace zl;

enum class AllocationErrorCode : uint8_t
{
    Okay,
    ResultReleased,
    OOM,
};

res<uint8_t *, AllocationErrorCode> alloc_bytes()
{
    auto *allocation = static_cast<uint8_t*>(malloc(500));
    if (!allocation)
        return AllocationErrorCode::OOM;
    else
        return allocation;
}

int main()
{
    auto maybe_bytes = alloc_bytes();
    if (!maybe_bytes.okay()) {
        printf("Allocation failure, errcode %u", uint8_t(maybe_bytes.status()));
        // could also switch (maybe_bytes.status())
        return 1;
    }

    auto *bytes = maybe_bytes.release();
    std::memset(bytes, 0, 500);

    // do some stuff with bytes here...

    free(bytes);
}
```

### `defer`

Defer is especially nice when you have a function with multiple places where
it can fail, and one success exit point. For example:

```cpp
using namespace zl;

opt<std::array<void *, 3>> getmems()
{
    void *first_mem = malloc(100);
    if (!first_mem)
        return {};
    defer free_first_mem([first_mem]() { free(first_mem); });

    void *second_mem = malloc(100);
    if (!second_mem)
        return {};
    defer free_second_mem([second_mem]() { free(second_mem); });

    void *third_mem = malloc(100);
    if (!third_mem)
        return {};

    // okay, all initialization is good, dont free anything
    free_first_mem.cancel();
    free_second_mem.cancel();

    return std::array<void *, 3>{first_mem, second_mem, third_mem};
}
```

### Non-failing factory functions with emplace_back

Ziglike provides the `make_back` function, which accepts a container which has
a `value_type` defined and a `emplace_back()` member function. An obvious example
is std::vector:

```cpp
struct factoryable_t
{
  private:
    int i;
    float j;

    inline constexpr factoryable_t(int _i, float _j) : i(_i), j(_j)
    {
    }

  public:
    static inline constexpr factoryable_t make(int i, float j)
    {
        return factoryable_t(i, j);
    }

    static inline constexpr factoryable_t make()
    {
        return factoryable_t(0, 0.0f);
    }
};

std::vector<factoryable_t> vec;
make_back(vec, 1, 0.4f);
make_back(vec);
```

This of course misses out on the opportunity to have potentially failing factory
functions, but unfortunately the stdlib APIs do not provide methods to cancel
emplace_back if construction fails, except by exceptions.

## Macros

- `ZIGLIKE_SLICE_NO_ITERATOR`: disable `#include <iterator>` and stdlib iterator functionality for `zl::slice`.
- `ZIGLIKE_NO_SMALL_OPTIONAL_SLICE`: in order to do some size optimization, opt includes `slice.h`. Define this macro to avoid the inclusion of the header. defining this macro will increase the size of opt<slice> types.
- `ZIGLIKE_OPTIONAL_ALLOW_POINTERS`: disable a static assert which stops you from putting pointers into an opt.
