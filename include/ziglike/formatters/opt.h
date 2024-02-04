#pragma once
#ifdef ZIGLIKE_USE_FMT
#include "../opt.h"
#include <fmt/core.h>

template <typename T> struct fmt::formatter<zl::opt<T>>
{
    static_assert(
        fmt::is_formattable<T>::value,
        "Attempt to format an optional whose contents is not formattable.");

    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();

        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");

        // just immediately return the iterator to the ending valid character
        return it;
    }

    format_context::iterator format(const zl::opt<T> &optional,
                                    format_context &ctx) const
    {
        if (optional.m_has_value) {
            if constexpr (zl::opt<T>::is_reference) {
                return fmt::format_to(ctx.out(), "{}",
                                      optional.m_value.some.item);
            } else {
                return fmt::format_to(ctx.out(), "{}", optional.m_value.some);
            }
        }
        return fmt::format_to(ctx.out(), "null");
    }
};
#endif
