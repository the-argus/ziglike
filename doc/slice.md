# zl::slice

`slice` is a template over one type. Its declaration looks like this:

```cpp
template <typename T>
class slice
```

- `slice` is formattable.

- `slice` is not default constructible.

- `slice` can be constructed from a STL container such as a `std::vector` or `std::array`. In order for something to be passed into the `slice` for construction, it must have `data()` and `size()` functions available.

- `slice` has a "subslice" constructor, where you pass in something (a `std::vector`, or another `zl::slice`) and then provide two numbers: `from` and `to`. `from` is the beginning index of the subslice in the container, inclusive, and `to` is the ending index, exclusive.

- A `slice` can be iterated over, both const and nonconst. For example:

```cpp
std::vector ints;
for (size_t i = 0; i < 20; i ++) {
    ints.push_back(i);
}

zl::slice ints_pointer(ints);
for (int integer : ints_pointer) {
    // do something with the integer
}

zl::slice half_the_ints(ints, 0, int.size() / 2);
for (int integer : half_the_ints) {
    // do something with the integer
}
```

## Methods

- `T* data()`

  - Returns a non-const pointer to the head of the data pointed at by the `slice`. This pointer is guaranteed to not be null.

- `const T* data() const`

  - const variant of previous function.

- `size_t size() const`

  - Returns the number of items the slice is pointing at.

## Friends

- `static slice<T> raw_slice(T& data, size_t size)`

  - Takes in `data`, which is a reference to the first item in an array of `T`. `size` is how many of those items you want to point at. Returns a slice that points at those things.
  - This is the only memory-unsafe part of `slice` (besides double-free and use-after-free potential due to raw pointer storage).
