#pragma once
#include <optional>
namespace glap::generators {
    namespace colors {
#ifdef _WIN32
        namespace foreground {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = 1;
            static constexpr int DARK_GREEN = 2;
            static constexpr int DARK_CYAN = 3;
            static constexpr int DARK_RED = 4;
            static constexpr int DARK_MAGENTA = 5;
            static constexpr int DARK_YELLOW = 6;
            static constexpr int GRAY = 7;
            static constexpr int DARK_GRAY = 8;
            static constexpr int BLUE = 9;
            static constexpr int GREEN = 10;
            static constexpr int CYAN = 11;
            static constexpr int RED = 12;
            static constexpr int MAGENTA = 13;
            static constexpr int YELLOW = 14;
            static constexpr int WHITE = 15;
        }
        namespace background {
            static constexpr int BLACK = 0;
            static constexpr int DARK_BLUE = 16;
            static constexpr int DARK_GREEN = 32;
            static constexpr int DARK_CYAN = 48;
            static constexpr int DARK_RED = 64;
            static constexpr int DARK_MAGENTA = 80;
            static constexpr int DARK_YELLOW = 96;
            static constexpr int GRAY = 112;
            static constexpr int DARK_GRAY = 128;
            static constexpr int BLUE = 144;
            static constexpr int GREEN = 160;
            static constexpr int CYAN = 176;
            static constexpr int RED = 192;
            static constexpr int MAGENTA = 208;
            static constexpr int YELLOW = 224;
            static constexpr int WHITE = 240;
        }
#else
        namespace foreground {
            static constexpr int BLACK = 30;
            static constexpr int RED = 31;
            static constexpr int GREEN = 32;
            static constexpr int YELLOW = 33;
            static constexpr int BLUE = 34;
            static constexpr int MAGENTA = 35;
            static constexpr int CYAN = 36;
            static constexpr int WHITE = 37;
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
        void apply() const noexcept;
        static void reset() noexcept;
    private:
        auto is_none() const noexcept -> bool {
            return !bold && !underlined && !italic && !foreground_color && !background_color;
        }
    };
}