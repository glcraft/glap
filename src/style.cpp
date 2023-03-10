#ifdef _WIN32
#include <Windows.h>
#endif
#include <string>
#include <iostream>
#include <glap/core/fmt.h>
#include <glap/generators/style.h>
namespace glap::generators {
#ifdef _WIN32
    void Style::apply() const noexcept {

    }
    void Style::reset() noexcept {
        
    }
#else
    void Style::apply() const noexcept
    {
        std::string style;
        auto it = std::back_inserter(style);
        if (bold) {
            format_to(it, "{};", *bold ? 1 : 22);
        }
        if (underlined) {
            format_to(it, "{};", *underlined ? 4 : 24);
        }
        if (italic) {
            format_to(it, "{};", *italic ? 3 : 23);
        }
        if (foreground_color) {
            format_to(it, "{};", *foreground_color);
        }
        if (background_color) {
            format_to(it, "{};", *background_color);
        }
        if (!style.empty()) {
            style.pop_back();
        }
        std::cout.write("\033[", 2);
        std::cout.write(std::data(style), std::size(style));
        std::cout.write("m", 1);
    }
    void Style::reset() noexcept {
        std::cout.write("\033[0m", 4);
    }
#endif
}

template <> struct fmt::formatter<glap::generators::Style> {
    constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(const glap::generators::Style& style, FormatContext& ctx)
    {
#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
        using fmt::format_to;
        format_to(ctx.out(), "\033[");
        [&ctx, sep = ""](auto&&... args) mutable {
            auto f = [&ctx, &sep](auto&& arg) mutable {
                if (arg != 0) {
                    format_to(ctx.out(), "{}{}", sep, arg);
                    sep = ";";
                }
            };
            (f(args), ...);
        }(
            style.bold ? (*style.bold ? 1 : 22) : 0,
            style.underlined ? (*style.underlined ? 4 : 24) : 0,
            style.italic ? (*style.italic ? 3 : 23) : 0,
            style.foreground_color ? *style.foreground_color : 0,
            style.background_color ? *style.background_color : 0
        );
        format_to(ctx.out(), "m");
        return ctx.out();
#else
        HANDLE h = GetStdHandle ( STD_OUTPUT_HANDLE );
        SetConsoleTextAttribute ( h, 
            style.foreground_color ? *style.foreground_color : 0 
            | style.background_color ? *style.background_color : 0
            | style.underlined && *style.underlined ? COMMON_LVB_UNDERSCORE : 0
        );
        return ctx.out();
#endif
    }
};