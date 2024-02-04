# ziglike

Some reimplementations of the Zig error union and optional types for C++ 17 and up

## Macros

- ZIGLIKE_SLICE_NO_ITERATOR: disable #include <iterator> and stdlib iterator functionality for zl::slice.
- ZIGLIKE_NO_SMALL_OPTIONAL_REFERENCE: in order to do some size optimization, opt includes slice.h. define this macro to avoid this, increasing the size of opt<slice> and opt<&> types.
- ZIGLIKE_OPTIONAL_ALLOW_POINTERS: disable a static assert which stops you from putting pointers into an opt.
