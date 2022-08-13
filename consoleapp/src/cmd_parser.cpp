#include <cmd_parser.h>
#include <fmt/format.h>
#include <string_view>

namespace cmd 
{
    
    auto result::Error::to_string() const -> std::string {
        auto constexpr types = std::array{
            " (type: argument)",
            " (type: flag)",
            " (type: command)",
            "",
            " (type: unknown)",
        };
        auto constexpr codes_text = std::array{
            "no argument",
            "missing argument",
            "missing flag",
            "no global command set",
            "bad command",
            "unknown parameter",
            "invalid value",
            "missing value",
            "flag with value",
            "parameter too used",
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