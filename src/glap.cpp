#include <glap.h>
#include <fmt/format.h>
#include <string_view>
#include <array>
#include <glap/common/fmt.h>
#include <glap/common/error.h>


namespace glap 
{
    auto Error::to_string() const -> std::string {
        auto constexpr types = std::array{
            " (type: command)",
            " (type: argument)",
            " (type: flag)",
            " (type: input)",
            "",
            " (type: unknown)",
        };
        auto constexpr codes_text = std::array{
            "no argument",
            "missing argument",
            "missing flag",
            "no global command set",
            "command not found",
            "unknown parameter",
            "invalid value",
            "missing value",
            "already set",
            "flag with value",
            "flag number exceeded",
            "flag number too low",
            "required parameter(s) missing",
            "syntax error",
            "bad string",
        };
        auto value = std::string{};
        if (this->value) {
            value = fmt::format(" (={})", *this->value);
        }
        return fmt::format("\"{}\"{}{} : {}", this->argument, value, types[static_cast<std::size_t>(this->type)], codes_text[static_cast<std::size_t>(this->code)]);
    }
}