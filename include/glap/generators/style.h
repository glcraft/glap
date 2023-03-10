#pragma once
#include <optional>
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
        void apply() const noexcept;
        static void reset() noexcept;
    private:
        auto is_none() const noexcept -> bool {
            return !bold && !underlined && !italic && !foreground_color && !background_color;
        }
    };
}