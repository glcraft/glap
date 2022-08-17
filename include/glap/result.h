#pragma once
#include <string_view>
#include <span>
#include "config.h"
#include "utils.h"
#include "fmt.h"

namespace glap::result 
{
    using Input = std::string_view;
    struct Argument {
        std::string_view name;
        std::string_view value;
        const glap::config::Argument& argument_parser;
    };
    struct Flag {
        std::string_view name;
        uint32_t occurrence;
        const glap::config::Flag& flag_parser;
    };
    using Parameter = std::variant<Argument, Flag, Input>;
    struct Command {
        std::string_view name;
        std::vector<Parameter> parameters;
        bool help;

        Flag& add_flag(const glap::config::Flag& flag) {
            auto found = std::find_if(parameters.begin(), parameters.end(), [name=flag.longname](const auto& parameter) {
                return std::holds_alternative<Flag>(parameter) && std::get<Flag>(parameter).name == name;
            });
            if (found == parameters.end()) {
                parameters.push_back(Flag{flag.longname, 1, flag});
                return std::get<Flag>(parameters.back());
            } else {
                std::get<Flag>(*found).occurrence++;
                return std::get<Flag>(*found);
            }
        }
    };
    struct Result {
        std::string_view program;
        Command command;
        // std::vector<Parameter> parameters;
    };
    struct Error {
        std::string_view argument;
        std::optional<std::string_view> value;
        enum class Type {
            Command,
            Argument,
            Flag,
            Input,
            None,
            Unknown
        } type;
        enum class Code {
            NoArgument,
            MissingArgument,
            MissingFlag,
            NoGlobalCommand,
            BadCommand,
            UnknownParameter,
            InvalidValue,
            MissingValue,
            FlagWithValue,
            TooManyFlags,
            NotEnoughFlags,
            RequiredArgument,
            SyntaxError,
            BadString
        } code;
        std::string to_string() const;
    };
    template <class T>
    using Expected = expected<T, result::Error>;
    struct PositionnedError {
        using difference_type = decltype(std::distance(std::span<std::string>().begin(), std::span<std::string>().end()));
        Error error;
        difference_type position;
        auto to_string() const {
            return glap::format("at argument {}: {}", position, error.to_string());
        }
    };
    template<class T>
    using PosExpected = expected<T, PositionnedError>;
    constexpr auto make_unexpected(PositionnedError error) {
        return unexpected<PositionnedError>(error);
    }
}