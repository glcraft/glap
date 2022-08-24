#pragma once
#include "expected.h"
#include "fmt.h"
#include <string_view>
#include <span>
#include <algorithm>
#include <optional>
namespace glap 
{
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
    using Expected = expected<T, Error>;
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
    template <class Err>
    constexpr auto make_unexpected(Err error) {
        return unexpected<Err>(error);
    }
}