#pragma once
#include <functional>
#include <fmt/format.h>
#include <iterator>
#include <stddef.h>
#include <limits>
#include <string>
#include <optional>
#include <tuple>
#include <variant>
#include <vector>
#include <span>
#include <string_view>
#include "expected.h"

namespace cmd
{
    namespace utils {
        template <typename T, typename V>
        concept Iterable = requires(T t) {
            {*t.begin()} -> std::convertible_to<V>;
            {*t.end()} -> std::convertible_to<V>;
        };
        template <typename T, typename V>
        concept Iterator = requires(T t) {
            {*t} -> std::convertible_to<V>;
            {++t} -> std::same_as<T&>;
        };

    }
    template <class CRTP>
    struct Common {
        std::string_view longname;
        std::optional<char32_t> shortname;
        std::string_view description;

        Common(std::string_view longname) : longname(longname), description()
        {}
        Common(std::string_view longname, char32_t shortname) : longname(longname), shortname(shortname), description()
        {}
        Common(std::string_view longname, std::optional<char32_t> shortname) : longname(longname), shortname(shortname), description()
        {}

        constexpr CRTP& set_longname(std::string_view longname) noexcept {
            this->longname = longname;
            return *static_cast<CRTP*>(this);
        }
        constexpr CRTP& set_shortname(char32_t shortname) noexcept {
            this->shortname = shortname;
            return *static_cast<CRTP*>(this);
        }
        constexpr CRTP& set_description(std::string_view description) noexcept {
            this->description = description;
            return *static_cast<CRTP*>(this);
        }
    };
    template<class CRTP>
    struct MinMax {
        // uint32_t min;
        uint32_t max;
        MinMax() : max(std::numeric_limits<decltype(this->max)>::max())
        {}
        MinMax(uint32_t max) : max(max)
        {}

        constexpr CRTP& set_max(int max) noexcept {
            this->max = max;
            return *static_cast<CRTP*>(this);
        }
    };

    struct Argument : Common<Argument> {
        std::optional<std::string_view> metavar;
        std::optional<std::function<bool(std::string_view)>> validator;
        std::optional<std::string_view> default_value = std::nullopt;
        bool is_required=false;

        Argument(std::string_view longname) : Common(longname), metavar(), validator(), default_value(), is_required(false)
        {}
        Argument(std::string_view longname, std::optional<char32_t> shortname) : Common(longname, shortname), metavar(), validator(), default_value(), is_required(false)
        {}
        Argument(const Argument&) = default;
        Argument(Argument&&) = default;
        Argument& operator=(const Argument&) = default;
        Argument& operator=(Argument&&) = default;
        ~Argument() = default;
        
        constexpr Argument& set_metavar(std::string_view metavar) noexcept {
            this->metavar = metavar;
            return *this;
        }
        Argument& set_validator(std::function<bool(std::string_view)> validator) noexcept {
            this->validator = validator;
            return *this;
        }
        constexpr Argument& set_default_value(std::string_view default_value) noexcept {
            this->default_value = default_value;
            return *this;
        }
        constexpr Argument& set_required(bool is_required) noexcept {
            this->is_required = is_required;
            return *this;
        }
    };
    struct Flag : Common<Flag>, MinMax<Flag> {
        Flag(std::string_view longname) : Common(longname), MinMax()
        {}
        Flag(std::string_view longname, std::optional<char32_t> shortname) : Common(longname, shortname), MinMax<Flag>()
        {}
        Flag(std::string_view longname, std::optional<char32_t> shortname, int max) : Common(longname, shortname), MinMax<Flag>(max)
        {}
        Flag(const Flag&) = default;
        Flag(Flag&&) = default;
        Flag& operator=(const Flag&) = default;
        Flag& operator=(Flag&&) = default;
        ~Flag() = default;
    };


    struct Command : Common<Command> {
        std::vector<Argument> arguments;
        std::vector<Flag> flags;
        // std::vector<Command> subcommands;

        Command(std::string_view longname) : Common(longname)
        {}
        Command(std::string_view longname, char32_t shortname) : Common(longname, shortname)
        {}
        Command(std::string_view longname, std::optional<char32_t> shortname) : Common(longname, shortname)
        {}
        Command(const Command&) = default;
        Command(Command&&) = default;
        Command& operator=(const Command&) = default;
        Command& operator=(Command&&) = default;
        ~Command() = default;

        Command& add_argument(Argument argument) {
            arguments.push_back(argument);
            return *this;
        }
        Command& add_flag(Flag flag) {
            flags.push_back(flag);
            return *this;
        }
        Argument& make_argument(std::string_view longname, std::optional<char32_t> shortname = std::nullopt) {
            return add_argument(Argument(longname, shortname)).arguments.back();
        }
        Flag& make_flag(std::string_view longname, std::optional<char32_t> shortname = std::nullopt) {
            return add_flag(Flag(longname, shortname)).flags.back();
        }
        // Command& add_subcommand(Command subcommand) {
        //     subcommands.push_back(subcommand);
        //     return *this;
        // }
    };

    namespace result 
    {
        struct Argument {
            std::string_view name;
            std::string_view value;
        };
        struct Flag {
            std::string_view name;
            uint32_t occurrence;
        };
        using Parameter = std::variant<Argument, Flag>;
        struct Command {
            std::string_view name;
            std::vector<Parameter> parameters;

            Flag& add_flag(std::string_view name) {
                auto found = std::find_if(parameters.begin(), parameters.end(), [&name](const auto& parameter) {
                    return std::holds_alternative<Flag>(parameter) && std::get<Flag>(parameter).name == name;
                });
                if (found == parameters.end()) {
                    parameters.push_back(Flag{name, 1});
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
            std::vector<Parameter> parameters;
        };
        struct Error {
            std::string_view argument;
            std::optional<std::string_view> value;
            enum class Type {
                Argument,
                Flag,
                Command,
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
                return fmt::format("at argument {}: {}", position, error.to_string());
            }
        };
        template<class T>
        using PosExpected = expected<T, PositionnedError>;
        constexpr auto make_unexpected(PositionnedError error) {
            return unexpected<PositionnedError>(error);
        }
    }

    class Parser {
        std::optional<std::variant<Command, std::string_view>> global_command;
        std::vector<Command> commands;
        std::string_view program_name;
    public:
        
        Parser& add_command(Command command) {
            commands.push_back(command);
            return *this;
        }
        Command& make_command(std::string_view longname, std::optional<char32_t> shortname = std::nullopt) {
            return add_command(Command(longname, shortname)).commands.back();
        }
        Parser& set_global_command(Command command) {
            global_command = command;
            return *this;
        }
        auto parse(utils::Iterable<std::string_view> auto args) const -> result::PosExpected<result::Result>;
    private:
        auto get_global_command() const -> std::optional<std::reference_wrapper<const Command>> {
            if (!global_command.has_value()) 
                return std::nullopt;
            
            if (std::holds_alternative<Command>(global_command.value())) 
                return std::optional{std::ref(std::get<Command>(global_command.value()))};

            for (const auto& command : commands) {
                if (std::get<std::string_view>(global_command.value()) == command.longname) {
                    return std::optional{std::ref(command)};
                }
            }
            return std::nullopt;
        }
        template <utils::Iterator<std::string_view> Iter>
        auto parse_long_argument(Iter& itarg, Iter end, const Command& command, result::Command& result_command) const -> result::PosExpected<bool>;
        template <utils::Iterator<std::string_view> Iter>
        auto parse_short_argument(Iter& itarg, Iter end, const Command& command, result::Command& result_command) const -> result::PosExpected<bool>;
        
        auto add_argument(result::Command& result_command, const Argument& argument, std::string_view name, const std::string_view value) const -> result::PosExpected<bool>;
        auto add_flag(result::Command& result_command, const Flag& flag, std::string_view name) const -> result::PosExpected<bool>;

        auto parse_command(utils::Iterable<std::string_view> auto args, const Command& command) const -> result::PosExpected<result::Command>;

    };
}

#include "cmd/cmd_parser.inl"