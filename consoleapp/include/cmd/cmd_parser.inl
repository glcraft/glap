#pragma once
#include "../cmd_parser.h"
#include "../utils.h"
#include <string_view>
namespace cmd
{
    auto Parser::parse(utils::Iterable<std::string_view> auto args) const -> result::Expected<result::Result> {
        result::Result result;
        result.program = args[0];

        auto itarg = std::next(args.begin());
        std::optional<Command> current_command = std::nullopt;
        // Find command
        if (std::string_view(*itarg).starts_with("-")) {
            current_command = this->get_global_command();
        } else {
            auto char_len = ::utils::uni::utf8_char_length(*itarg);
            if (!char_len) 
                return unexpected<result::Error>(char_len.error());
            auto found_cmd = commands.end();
            if (char_len.value() <= std::string_view(*itarg).size()) {
                auto codepoint = ::utils::uni::codepoint(*itarg).value();
                found_cmd = std::find_if(commands.begin(), commands.end(), [codepoint](const Command& command) {
                    return command.shortname == codepoint;
                });
            } else {
                found_cmd = std::find_if(commands.begin(), commands.end(), [&](const Command& command) {
                    return command.longname == *itarg;
                });
            }
            if (found_cmd != commands.end()) {
                current_command = *found_cmd;
            } else {
                return unexpected<result::Error>(result::Error{
                    .argument = *itarg,
                    .value = std::nullopt,
                    .type = result::Error::Type::None,
                    .code = result::Error::Code::BadCommand
                });
            }
            if (!current_command)
                return unexpected<result::Error>(result::Error{
                    .argument = *itarg,
                    .value = std::nullopt,
                    .type = result::Error::Type::None,
                    .code = result::Error::Code::MissingCommand
                });
            ++itarg;
            auto parsed_cmd = parse_command(args.subspan(std::distance(args.begin(), itarg)), current_command.value());
        }
        // Parse command arguments
    }
    auto Parser::parse_argument(utils::Iterable<std::string_view> auto args) const -> PosExpected<result::Argument>
    {}
    auto Parser::parse_flag(utils::Iterable<std::string_view> auto args) const -> PosExpected<result::Flag>
    {}
    auto Parser::parse_command(utils::Iterable<std::string_view> auto args, Command& command) const -> PosExpected<result::Command> {
        result::Command result_command;
        command.longname = command.longname;
        command.shortname = command.shortname;

        for (auto itarg = std::next(args.begin()); itarg != args.end(); itarg++) {
            auto name = std::string_view(*itarg);
            bool is_short = false;
            if (name.starts_with("---")) {
                return unexpected<result::Error>(result::Error{
                    *itarg,
                    std::nullopt,
                    result::Error::Type::None,
                    result::Error::Code::SyntaxError
                });
            }
            if (name.starts_with("--")) {
                name = name.substr(2);
            } else if (name.starts_with("-")) {
                name = name.substr(1);
                is_short = true;
            }
            // auto found_flag = std::find_if(
            //     this->global_command.begin(),
            //     result.flags.end(),
            //     [&name](const result::Flag& flag) {
            //         return flag.name == name;
            //     }
            // );
        }
    }
}