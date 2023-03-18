#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>
#include <iostream>
#if __has_include(<fmt/format.h>)
#include <fmt/format.h>
#endif
#if __has_include(<format>) && __cpp_lib_format >= 201907L
#include <format>
#endif
#include <glap/generators/style.h>
namespace glap::generators {
#ifdef _WIN32
    void Style::reset() noexcept {
        
    }
#else
    void Style::reset() noexcept {
        std::cout.write("\033[0m", 4);
    }
#endif
}


#if __has_include(<fmt/format.h>)
template <> struct ::fmt::formatter<glap::generators::Style> {
    constexpr auto parse(format_parse_context& ctx) { 
        return ctx.begin(); 
    }
    template <typename FormatContext>
    auto format(const glap::generators::Style& style, FormatContext& ctx) {
        style.format(ctx);
    }
};
#endif
#if __has_include(<format>) && __cpp_lib_format >= 201907L	
template <> struct ::std::formatter<glap::generators::Style, char> {
    constexpr auto parse(format_parse_context& ctx) { 
        return ctx.begin(); 
    }
    template <typename FormatContext>
    auto format(const glap::generators::Style& style, FormatContext& ctx) {
        style.format(ctx);
    }
};
#endif