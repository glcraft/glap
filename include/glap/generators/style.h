#pragma once
#include <optional>
#ifdef _WIN32
#include <Windows.h>
#endif
#if __has_include(<fmt/format.h>)
#include <fmt/format.h>
#endif
#if __has_include(<format>) && __cpp_lib_format >= 201907L
#include <format>
#endif

namespace glap::generators {
    namespace colors {
#ifdef _WIN32
        namespace foreground {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = FOREGROUND_BLUE;
            static constexpr int DARK_GREEN = FOREGROUND_GREEN;
            static constexpr int DARK_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
            static constexpr int DARK_RED = FOREGROUND_RED;
            static constexpr int DARK_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE;
            static constexpr int DARK_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
            static constexpr int GRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            static constexpr int DARK_GRAY = FOREGROUND_INTENSITY;
            static constexpr int BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            static constexpr int CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
            static constexpr int MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            static constexpr int WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        }
        namespace background {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = BACKGROUND_BLUE;
            static constexpr int DARK_GREEN = BACKGROUND_GREEN;
            static constexpr int DARK_CYAN = BACKGROUND_BLUE | BACKGROUND_GREEN;
            static constexpr int DARK_RED = BACKGROUND_RED;
            static constexpr int DARK_MAGENTA = BACKGROUND_BLUE | BACKGROUND_RED;
            static constexpr int DARK_YELLOW = BACKGROUND_GREEN | BACKGROUND_RED;
            static constexpr int GRAY = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
            static constexpr int DARK_GRAY = FOREGROUND_INTENSITY;
            static constexpr int BLUE = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
            static constexpr int GREEN = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
            static constexpr int CYAN = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
            static constexpr int RED = BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int MAGENTA = BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int YELLOW = BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int WHITE = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
        }
#else
        namespace foreground {
            static constexpr int BLACK = 30;
            static constexpr int DARK_BLUE = 34;
            static constexpr int RED = 31;
            static constexpr int GREEN = 32;
            static constexpr int YELLOW = 33;
            static constexpr int BLUE = 34;
            static constexpr int MAGENTA = 35;
            static constexpr int CYAN = 36;
            static constexpr int WHITE = 37;
            static constexpr int DARK_GRAY = 90;
            
        }
        namespace background {
            static constexpr int BLACK = 40;
            static constexpr int RED = 41;
            static constexpr int GREEN = 42;
            static constexpr int YELLOW = 43;
            static constexpr int BLUE = 44;
            static constexpr int MAGENTA = 45;
            static constexpr int CYAN = 46;
            static constexpr int WHITE = 47;
        }
#endif
    }
    struct Style {
        std::optional<int> foreground_color = std::nullopt;
        std::optional<int> background_color = std::nullopt;
        std::optional<bool> bold = std::nullopt;
        std::optional<bool> underlined = std::nullopt;
        std::optional<bool> italic = std::nullopt;
        template <typename FormatContext>
        auto format(FormatContext& ctx) const 
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
            this->bold ? (*this->bold ? 1 : 22) : 0,
            this->underlined ? (*this->underlined ? 4 : 24) : 0,
            this->italic ? (*this->italic ? 3 : 23) : 0,
            this->foreground_color ? *this->foreground_color : 0,
            this->background_color ? *this->background_color : 0
        );
        format_to(ctx.out(), "m");
        return ctx.out();
#else
        HANDLE h = GetStdHandle ( STD_OUTPUT_HANDLE );
        SetConsoleTextAttribute ( h, 
            (this->foreground_color ? *this->foreground_color : 0) 
            | (this->background_color ? *this->background_color : 0)
            | (this->underlined && *this->underlined ? COMMON_LVB_UNDERSCORE : 0)
        );
        return ctx.out();
#endif
        }
        static void reset() noexcept;
    private:
        auto is_none() const noexcept -> bool {
            return !bold && !underlined && !italic && !foreground_color && !background_color;
        }
    };

}
namespace glap::style {
    namespace attributes {
    #ifdef _WIN32
        static constexpr int UNDERLINE = COMMON_LVB_UNDERSCORE;
        static constexpr int BOLD = 0; // bold is not supported on windows
        static constexpr int ITALIC = 0; // italic is not supported on windows
        namespace foreground {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = FOREGROUND_BLUE;
            static constexpr int DARK_GREEN = FOREGROUND_GREEN;
            static constexpr int DARK_CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE;
            static constexpr int DARK_RED = FOREGROUND_RED;
            static constexpr int DARK_MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE;
            static constexpr int DARK_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN;
            static constexpr int GRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            static constexpr int DARK_GRAY = FOREGROUND_INTENSITY;
            static constexpr int BLUE = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int GREEN = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            static constexpr int CYAN = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int RED = FOREGROUND_RED | FOREGROUND_INTENSITY;
            static constexpr int MAGENTA = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
            static constexpr int YELLOW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            static constexpr int WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
        }
        namespace background {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = BACKGROUND_BLUE;
            static constexpr int DARK_GREEN = BACKGROUND_GREEN;
            static constexpr int DARK_CYAN = BACKGROUND_BLUE | BACKGROUND_GREEN;
            static constexpr int DARK_RED = BACKGROUND_RED;
            static constexpr int DARK_MAGENTA = BACKGROUND_BLUE | BACKGROUND_RED;
            static constexpr int DARK_YELLOW = BACKGROUND_GREEN | BACKGROUND_RED;
            static constexpr int GRAY = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
            static constexpr int DARK_GRAY = FOREGROUND_INTENSITY;
            static constexpr int BLUE = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
            static constexpr int GREEN = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
            static constexpr int CYAN = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
            static constexpr int RED = BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int MAGENTA = BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int YELLOW = BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
            static constexpr int WHITE = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY;
        }
#else
    static constexpr int BOLD = 1;
    static constexpr int UNDERLINED = 4;
    static constexpr int ITALIC = 3;
        namespace foreground {
            static constexpr int BLACK = 30;
            static constexpr int DARK_BLUE = 34;
            static constexpr int RED = 31;
            static constexpr int GREEN = 32;
            static constexpr int YELLOW = 33;
            static constexpr int BLUE = 34;
            static constexpr int MAGENTA = 35;
            static constexpr int CYAN = 36;
            static constexpr int WHITE = 37;
            static constexpr int DARK_GRAY = 90;
            
        }
        namespace background {
            static constexpr int BLACK = 40;
            static constexpr int RED = 41;
            static constexpr int GREEN = 42;
            static constexpr int YELLOW = 43;
            static constexpr int BLUE = 44;
            static constexpr int MAGENTA = 45;
            static constexpr int CYAN = 46;
            static constexpr int WHITE = 47;
        }
#endif
}
    template <auto ...Attributes>
    struct Style2 {
        template <typename FormatContext>
        auto format(FormatContext& ctx) const -> decltype(ctx.out())
        {
#ifdef _WIN32
            HANDLE h = GetStdHandle ( STD_OUTPUT_HANDLE );
            if constexpr (sizeof...(Attributes) > 0) 
                SetConsoleTextAttribute(h, (Attributes | ...));
            else
                SetConsoleTextAttribute(h, 0);
#else
            format_to(ctx.out(), "\033[");
            if constexpr (sizeof...(Attributes) > 0) 
                this->template format_to<Attributes...>(ctx);
            else 
                format_to(ctx.out(), "0");
            format_to(ctx.out(), "m");
#endif
            return ctx.out();
        }
    private:
#ifndef _WIN32
        template <typename FormatContext, auto Attr, auto ...Attrs>
        auto format_to(FormatContext& ctx) const -> void
        {
            if constexpr(sizeof...(Attrs) == sizeof...(Attributes) - 1) {
                format_to(ctx.out(), "{}", Attr);
            } else {
                format_to(ctx.out(), ";{}", Attr);
            }
            if constexpr(sizeof...(Attrs) > 0) 
                this->template format_to<FormatContext, Attrs...>(ctx);
        }
#endif
    };
    template <auto ...Attributes>
    static constexpr auto STYLE = Style2<Attributes...>{};
}

#if __has_include(<fmt/format.h>)
template <auto ...Attributes> struct ::fmt::formatter<glap::style::Style2<Attributes...>> {
    constexpr auto parse(format_parse_context& ctx) { 
        return ctx.begin(); 
    }
    template <typename FormatContext>
    auto format(const glap::style::Style2<Attributes...>& style, FormatContext& ctx) {
        return style.format(ctx);
    }
};
#endif
#if __has_include(<format>) && __cpp_lib_format >= 201907L	
template <auto ...Attributes> struct ::std::formatter<glap::style::Style2<Attributes...>, char> {
    constexpr auto parse(format_parse_context& ctx) { 
        return ctx.begin(); 
    }
    template <typename FormatContext>
    auto format(const glap::style::Style2<Attributes...>& style, FormatContext& ctx) {
        return style.format(ctx);
    }
};
#endif