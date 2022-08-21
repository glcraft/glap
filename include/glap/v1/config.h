#pragma once
#include <string_view>
#include <optional>
#include <functional>
#include <vector>
namespace glap::config
{
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
        bool required=false;

        Argument(std::string_view longname) : Common(longname), metavar(), validator(), default_value(), required(false)
        {}
        Argument(std::string_view longname, std::optional<char32_t> shortname) : Common(longname, shortname), metavar(), validator(), default_value(), required(false)
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
            this->required = is_required;
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
        std::optional<std::function<bool(std::string_view)>> input_validator;
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
        Command& set_input_validator(std::function<bool(std::string_view)> validator) {
            input_validator = validator;
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
}