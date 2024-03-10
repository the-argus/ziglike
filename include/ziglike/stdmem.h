#pragma once

#include "ziglike/slice.h"
#include <cstdint>
#include <cstring>

namespace zl {
/// Copy the contents of source into destination, byte by byte, without invoking
/// any copy constructors or destructors.
/// T must be trivially copyable.
/// If the two slices are not the same size, or if they overlap, the function
/// returns false and does nothing. Otherwise, it returns true.
template <typename T>
constexpr bool memcopy(zl::slice<T> destination, zl::slice<T> source) noexcept;

/// Identical to memcopy, except it allows for sources which are smaller than
/// the destination. It will also allow copying of non-trivially-copyable types.
template <typename T>
constexpr bool memcopy_lenient(zl::slice<T> destination,
                               zl::slice<T> source) noexcept;

/// Compare two slices of memory, byte by byte, without invoking any == operator
/// overloads.
/// The memory can overlap and alias (in the latter case the function just
/// returns true).
/// If the two slices of memory are differently size, the function immediately
/// returns false;
template <typename T>
constexpr bool memcompare(zl::slice<T> memory_1,
                          zl::slice<T> memory_2) noexcept;

/// Check if slice "inner" points only to items also pointed at by slice "outer"
template <typename T>
constexpr bool memcontains(zl::slice<T> outer, zl::slice<T> inner) noexcept;

/// Check if two slices of memory have any memory in common.
template <typename T>
constexpr bool memoverlaps(zl::slice<T> a, zl::slice<T> b) noexcept;

/// Fills a block of memory of type T by copying an instance of T into every
/// spot in that memory. T must be nothrow copy constructible. Does not invoke
/// destructors of any items already in the memory.
template <typename T>
constexpr void memfill(zl::slice<T> slice, T original) noexcept;
} // namespace zl

template <typename T>
inline constexpr bool zl::memcopy(zl::slice<T> destination,
                                  zl::slice<T> source) noexcept
{
    static_assert(std::is_trivially_copyable_v<T>,
                  "Cannot copy non-trivially copyable type.");
    if (destination.size() != source.size() ||
        memoverlaps(destination, source)) {
        return false;
    }
    return memcopy_lenient(destination, source);
}

template <typename T>
inline constexpr bool zl::memcopy_lenient(zl::slice<T> destination,
                                          zl::slice<T> source) noexcept
{
    if (destination.size() < source.size() ||
        memoverlaps(destination, source)) {
        return false;
    }

    std::memcpy(destination.data(), source.data(), source.size() * sizeof(T));
    return true;
}

template <typename T>
inline constexpr bool zl::memcompare(zl::slice<T> memory_1,
                                     zl::slice<T> memory_2) noexcept
{
    if (memory_1.size() != memory_2.size()) {
        return false;
    }
    if (memory_1.data() == memory_2.data()) {
        return true;
    }
    const size_t size = memory_1.size() * sizeof(T);
    const auto *const as_bytes_1 =
        reinterpret_cast<const uint8_t *>(memory_1.data());
    const auto *const as_bytes_2 =
        reinterpret_cast<const uint8_t *>(memory_2.data());

    for (size_t i = 0; i < size; ++i) {
        if (as_bytes_1[i] != as_bytes_2[i]) {
            return false;
        }
    }
    return true;
}

template <typename T>
inline constexpr bool zl::memcontains(zl::slice<T> outer,
                                      zl::slice<T> inner) noexcept
{
    return outer.begin().ptr() <= inner.begin().ptr() &&
           outer.end().ptr() >= inner.end().ptr();
}

template <typename T>
inline constexpr bool zl::memoverlaps(zl::slice<T> a, zl::slice<T> b) noexcept
{
    return a.begin().ptr() < b.end().ptr() && b.begin().ptr() < a.end().ptr();
}

template <typename T>
inline constexpr void zl::memfill(zl::slice<T> slice, const T original) noexcept
{
    static_assert(
        std::is_nothrow_copy_constructible_v<T>,
        "Cannot memfill a type which can throw when copy constructed.");
    for (T &item : slice) {
        new ((void *)std::addressof(item)) T(original);
    }
}

template <>
inline constexpr void zl::memfill(zl::slice<uint8_t> slice,
                                  const uint8_t original) noexcept
{
    std::memset(slice.data(), original, slice.size());
}
