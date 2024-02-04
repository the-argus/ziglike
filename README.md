# ziglike

Some reimplementations of the Zig error union and optional types for C++ 17 and up.

Header-only.

Could also be considered a sort of C++17 backport of std::expected and std::span.

## Documentation

- [`zl::res`](./doc/res.md) : replaces exceptions with minimal overhead and no footguns, using error-code enums.
- [`zl::opt`](./doc/opt.md) : wraps a type and makes it nullable. Similar to std::optional, however it has slightly different semantics and supports reference types. An `opt<T&>` is the same size as a `T*`.
- [`zl::slice`](./doc/slice.md) : a struct which has a pointer to an array, and a `size_t` number of things. Very similar to `std::span`, but its non-nullable. Also, it works with C++17. An `opt<slice<T>>` is the same size as a `slice<T>`.

## Macros

- `ZIGLIKE_SLICE_NO_ITERATOR`: disable `#include <iterator>` and stdlib iterator functionality for `zl::slice`.
- `ZIGLIKE_NO_SMALL_OPTIONAL_SLICE`: in order to do some size optimization, opt includes `slice.h`. Define this macro to avoid the inclusion of the header. defining this macro will increase the size of opt<slice> types.
- `ZIGLIKE_OPTIONAL_ALLOW_POINTERS`: disable a static assert which stops you from putting pointers into an opt.
