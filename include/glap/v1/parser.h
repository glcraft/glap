#pragma once
#include "../common/utils.h"
#include "../common/error.h"
#include "config.h"
#include "result.h"

namespace glap
{
    class Parser {
        std::optional<std::variant<config::Command, std::string_view>> global_command;
        std::vector<config::Command> commands;
        std::string_view program_name;
    public:
        
        Parser& add_command(config::Command command) {
            commands.push_back(command);
            return *this;
        }
        config::Command& make_command(std::string_view longname, std::optional<char32_t> shortname = std::nullopt) {
            return add_command(config::Command(longname, shortname)).commands.back();
        }
        Parser& set_global_command(config::Command command) {
            global_command = command;
            return *this;
        }
        Parser& set_global_command(std::string_view longname) {
            global_command = longname;
            return *this;
        }
        auto parse(utils::Iterable<std::string_view> auto args) const -> PosExpected<result::Result>;
    private:
        auto get_global_command() const -> std::optional<std::reference_wrapper<const config::Command>> {
            if (!global_command.has_value()) 
                return std::nullopt;
            
            if (std::holds_alternative<config::Command>(global_command.value())) 
                return std::optional{std::ref(std::get<config::Command>(global_command.value()))};

            for (const auto& command : commands) {
                if (std::get<std::string_view>(global_command.value()) == command.longname) {
                    return std::optional{std::ref(command)};
                }
            }
            return std::nullopt;
        }
        template <utils::Iterator<std::string_view> Iter>
        auto parse_long_argument(Iter& itarg, Iter end, const config::Command& command, result::Command& result_command) const -> PosExpected<bool>;
        template <utils::Iterator<std::string_view> Iter>
        auto parse_short_argument(Iter& itarg, Iter end, const config::Command& command, result::Command& result_command) const -> PosExpected<bool>;
        
        auto add_argument(result::Command& result_command, const config::Argument& argument, std::string_view name, std::string_view value) const -> PosExpected<bool>;
        auto add_flag(result::Command& result_command, const config::Flag& flag, std::string_view name) const -> PosExpected<bool>;
        auto add_input(result::Command& result_command, const config::Command& command, std::string_view input) const -> PosExpected<bool>;

        auto parse_command(utils::Iterable<std::string_view> auto args, const config::Command& command) const -> PosExpected<result::Command>;

    };
}

#include "parser.inl"