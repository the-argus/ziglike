# zl::opt

`opt` is a template over one type. Its declaration looks like this:

```cpp
template <typename T>
class opt
```

- `opt` is formattable, if `T` is formattable.

- `opt` is guaranteed to be only one byte larger than `T`.

- an `opt<Type &>` is unfortunately one byte larger than a `Type*`, at time of writing. Can be fixed but atm it is better to use pointers as nullable references, though potentially more bugprone.

- `opt` is moveable, if `T` is not a reference type and `T` is moveable.

- `opt` is always copyable, unless `T` is not a reference type and not trivially copyable.

- `opt` is contextually convertible to bool. That means that you can do the following:

```cpp
zl::opt<int> maybe_int;
if (maybe_int) { // contextually converted to boolean!
    fmt::println("optional int is present: {}", maybe_int.value());
} else {
    fmt::println("no optional int found, aborting");
    std::abort();
}
```

## Type Constraints

`T` must be either an lvalue reference or be a non-reference type which is nothrow destructible.

## Methods

- `bool has_value() const`
  
  - Returns true if the optional is not null, otherwise false.

- `void reset()`
  
  - Calls the destructor of the type stored inside the optional, and it becomes null.

- `void emplace(Args &&... args)`
  
  - Constructs an item of type `T` inside the optional. `Args` are the arguments you want to pass to the constructor of the `T` being initialized.

- `T& value()` (note: only available if `T` is not reference type)
  
  - Returns a reference to the item inside the optional. Do not store this reference for longer than the optional exists.

- `const T& value() const` (note: only available if `T` is not a reference type)
  
  - Const variant of previous `value()` function for non-reference types.

- `T value()` (note: only available if `T` is a reference type)
  
  - Returns a copy of the reference stored inside the optional.

- `const T value() const` (note: only available if `T` is a reference type)
  
  - Returns a const version copy of the reference stored inside the optional.
