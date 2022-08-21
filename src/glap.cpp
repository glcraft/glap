#include <glap.h>
#include <fmt/format.h>
#include <string_view>
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
    auto Parser::add_argument(result::Command& result_command, const config::Argument& argument, std::string_view name, const std::string_view value) const -> PosExpected<bool>
    {
        if (argument.validator.has_value() && !argument.validator.value()(value))
            return make_unexpected(PositionnedError{
                .error = Error{
                    .argument = name,
                    .value = value,
                    .type = Error::Type::Argument,
                    .code = Error::Code::InvalidValue
                },
                .position = 0
            });
        result_command.parameters.push_back(result::Parameter{
            result::Argument{
                .name = argument.longname,
                .value = value,
                .argument_parser = argument
            }
        });
        return true;
    }
    auto Parser::add_flag(result::Command& result_command, const config::Flag& flag, std::string_view name) const -> PosExpected<bool>
    {
        if (result_command.add_flag(flag.longname).occurrence > flag.max)
            return make_unexpected(PositionnedError{
                .error = Error{
                    .argument = name,
                    .value = std::nullopt,
                    .type = Error::Type::Flag,
                    .code = Error::Code::TooManyFlags
                },
                .position = 0
            });
        return true;
    }
    auto Parser::add_input(result::Command& result_command, const config::Command& command, std::string_view input) const -> PosExpected<bool>
    {
        if (command.input_validator && !command.input_validator.value()(input)) {
            return make_unexpected(PositionnedError{
                .error = Error{
                    .argument = input,
                    .value = std::nullopt,
                    .type = Error::Type::Input,
                    .code = Error::Code::InvalidValue
                },
                .position = 0
            });
        }
        result_command.parameters.push_back(input);
        return true;
    }
}