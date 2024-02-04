#pragma once
#ifdef ZIGLIKE_USE_FMT
#include "../res.h"
#include <fmt/core.h>

template <typename T, typename StatusCode>
struct fmt::formatter<zl::res<T, StatusCode>>
{
    static_assert(
        fmt::is_formattable<T>::value,
        "Attempt to format a zl::res whose contents are not formattable.");
    constexpr format_parse_context::iterator parse(format_parse_context &ctx)
    {
        auto it = ctx.begin();
        // first character should just be closing brackets since we dont allow
        // anything else
        if (it != ctx.end() && *it != '}')
            throw_format_error("invalid format");
        return it;
    }

    format_context::iterator format(const zl::res<T, StatusCode> &result,
                                    format_context &ctx) const
    {
        if (result.okay()) {
            if constexpr (zl::res<T, StatusCode>::is_reference) {
                return fmt::format_to(ctx.out(), "{}",
                                      result.m_value.some.item);
            } else {
                return fmt::format_to(ctx.out(), "{}", result.m_value.some);
            }
        } else {
            if constexpr (fmt::is_formattable<
                              decltype(result.m_status)>::value) {
                return fmt::format_to(ctx.out(), "err {}", result.m_status);
            } else {
                return fmt::format_to(
                    ctx.out(), "err {}",
                    std::underlying_type_t<decltype(result.m_status)>(
                        result.m_status));
            }
        }
    }
};
#endif
