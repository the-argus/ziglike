#pragma once
#ifdef ZIGLIKE_USE_FMT
#include "../slice.h"
#include <fmt/core.h>

template <typename T> struct fmt::formatter<zl::slice<T>>
{
    constexpr auto parse(format_parse_context &ctx)
        -> format_parse_context::iterator
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    auto format(const zl::slice<T> &slice, format_context &ctx) const
        -> format_context::iterator
    {
        return fmt::format_to(ctx.out(), "[{:p} -> {}]",
                              reinterpret_cast<void *>(slice.m_data),
                              slice.m_elements);
    }
};
#endif
