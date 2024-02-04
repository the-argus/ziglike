# ziglike

Some reimplementations of the Zig error union and optional types for C++ 17 and up

## Macros

- ZIGLIKE_SLICE_NO_ITERATOR: disable #include <iterator> and stdlib iterator functionality for zl::slice.
- ZIGLIKE_NO_SMALL_OPTIONAL_SLICE: in order to do some size optimization, opt includes slice.h. define this macro to avoid the inclusion of the header. defining this macro will increase the size of opt<slice> types.
- ZIGLIKE_OPTIONAL_ALLOW_POINTERS: disable a static assert which stops you from putting pointers into an opt.
