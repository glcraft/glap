#include <string_view>
#include <array>
#include <glap/core/fmt.h>
#include <glap/core/error.h>


namespace glap 
{
    auto Error::to_string() const -> std::string {
        auto constexpr types = std::array{
            " (type: command)",
            " (type: parameter)",
            " (type: flag)",
            " (type: input)",
            "",
            " (type: unknown)",
        };
        auto constexpr codes_text = std::array{
            "no parameter",
            "no global command set",
            "command not found",
            "unknown argument",
            "bad resolution",
            "bad validation",
            "duplicate parameter",
            "too many parameters",
            "missing value",
            "syntax error",
            "bad string",
        };
        auto value = std::string{};
        if (this->value) {
            value = glap::format(" (={})", *this->value);
        }
        return glap::format("\"{}\"{}{} : {}", this->parameter, value, types[static_cast<std::size_t>(this->type)], codes_text[static_cast<std::size_t>(this->code)]);
    }
}