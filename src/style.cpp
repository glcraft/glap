#include <glap/generators/style.h>
#include <string>
#include <glap/core/fmt.h>
#include <iostream>
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