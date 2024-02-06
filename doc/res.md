# zl::res

`res` is a template of two types: a payload and an error code enum. In the code, the payload is called type `T` and the error code is `StatusCode` Its declaration looks like this:

```cpp
template <typename T, typename StatusCode>
class res
```

General notes:

- `res` is formattable, provided that `T` is formattable.

- `res` is move constructible, provided that `T` is not a reference type and is move constructible.

- `res` is copy constructible, provided that `T` is trivially copyable or a reference type.

- `res` is never move or copy assignable: you can only create one and then release it later. To perform an "assignment," you can `std::move` the value returned by `release` into the constructor of another `res`.

- A `res` is guaranteed to be only one byte larger than `T`.

- A `res` is not threadsafe: it is intended to be used by only one thread as a return value from a function.

## Type constraints

- `T`: must either be an lvalue reference type or a type which is either move constructible or trivially copyable. Examples are `int`, `std::vector`, or `size_t*`

- `StatusCode`: must be an enum which is only one byte in size. It also must have two entries: one called `Okay` and one called `ResultReleased`. Further, the `Okay` entry must be the first element in the enum. Here is an example of a `StatusCode`:

```cpp
enum class MemoryAllocationErrorCode : uint8_t {
    Okay,
    ResultReleased,
    OOM,
};
```

## Methods

- `bool okay() const`

  - Returns true if the result's payload is valid and its not an error.
  - Returns false if the result contains an error.
  - Shortcut for `status() == StatusCode::Okay`

- `StatusCode status() const`

  - Returns the statuscode contained by the result.
  - Always valid to call this function.
  - If the result is not an error, this will be equal to `StatusCode::Okay`.

- `T && release()` (note: only available if `T` is not a reference type)

  - If the result is an error, this will print an error message and crash the program. Otherwise, it will return the item inside the result. It also sets the error code inside the result to `ResultReleased`.
  - Do not call this function twice: the result becomes an error after you remove the valid item from it.
  - Use this to convert the big evil result type like `zl::res<size_t*, MemoryAllocationErrorCode>` into a `size_t*`.
  - Always make sure that `okay()` returns true before calling this function.

- `T release()` (note: only available if `T` is a reference type)

  - Same as the other `release` function. The only difference is that it does not apply the `&&` to the return type, which makes it valid in the case that `T` is a reference type (ie. it already has a `&` on it)

- `T& release_ref` (note: only available if `T` is not a reference type)

  - A variant of `T& release()` which does not move, copy, nor destroy the item inside the result. Instead, it returns a reference to it.
  - IT IS UNDEFINED BEHAVIOR TO STORE A REFERENCE RETURNED BY RELEASE_REF FOR LONGER THAN YOU KEEP THE RESULT THAT PRODUCED IT.
  - This function does crash the program with an error message if you try to call it on an invalid result.
  - Intended to avoid a copy or move of a large item in a result.
  - Generally avoid this unless your types are really big and you're noticing a performance hit. It's not memory safe.

## Static Methods

- `res make(Args...)`

  - Factory function which directly constructs something into a result and then returns that result. Useful if something has an expensive move constructor and you don't want to have to construct it outside the result, and then move it into it.
  - Still ultimately moves the item once, to return it from the function. Theoretically could help to elide a single move in some cases.
