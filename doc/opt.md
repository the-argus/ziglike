# zl::opt

`opt` is a template over one type. Its declaration looks like this:

```cpp
template <typename T>
class opt
```

- `opt` is formattable, if `T` is formattable.

- `opt` is guaranteed to be only one byte larger than `T`. In the case of pointers and slices, it is the same size as `T`.

- `opt` is moveable, if `T` is not a reference type and `T` is moveable.

- `opt` is always copyable, unless `T` is not a reference type and not trivially copyable.

- `opt` is NOT contextually convertible to bool, unlike `std::optional`.

```cpp
zl::opt<int> maybe_int;
// not allowed
if (maybe_int) {
    ...
}

// do this instead
if (maybe_int.has_value()) {
    ...
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

- `const T& value_const() const` (note: only available if `T` is not a reference type)

  - Same as the const `value()` but explicit.

- `T value()` (note: only available if `T` is a reference type)

  - Returns a copy of the reference stored inside the optional.

- `const T value() const` (note: only available if `T` is a reference type)

  - Returns a const version copy of the reference stored inside the optional.

- `const T value_const() const` (note: only available if `T` is a reference type)

  - Same as `const T value() const` but explicit.

- `bool loose_compare(const T& other) const` (note: only available if `T` is a reference type)

  - Compares the thing that the optional is referencing to the item passed in. So these items may be different locations in memory, but if their == operator returns true, then so does `loose_compare`.
  - If this optional is null, then the function returns false.

- `bool loose_compare(const opt<T>& other) const` (note: only available if `T` is a reference type)

  - Identical to the standard `loose_compare` except `T` is replaced by another optional that contains `T`.
  - If both optionals are null, this returns true.

- `bool strict_compare(const T& other) const` (note: only available if `T` is a reference type)

  - Compares the address of the reference stored inside the optional to the address of the item passed in. If they are both references to the same item in memory, this returns true. If not, or if the optional is null, this returns false.

- `bool strict_compare(const opt<T>& other) const` (note: only available if `T` is a reference type)

  - Identical to the standard `strict_compare` except `T` is replaced by another optional that contains `T`.
  - If both optionals are null, this returns true.

- `friend bool operator==(const opt<T> &self, const opt<T> &other)` (note: only available if `T` is _not_ a reference type)

  - Allows comparison operator between two optionals of the same type, provided is it not a reference type. It will call the comparison operator of `T`.
  - If `self` and `other` are both null, this function returns true.
  - If `self` and `other` have different null-ness, this function returns false.
  - A mirror-image `operator!=` is provided.

- `bool operator==(const T &other) const` (note: only available if `T` is _not_ a reference type)

  - Allows comparison operator between an optional an something of the same type that it holds, provided is it not a reference type. It will call the comparison operator of `T`.
  - If the optional is null, this function returns false.
  - A mirror-image `operator!=` is provided.
