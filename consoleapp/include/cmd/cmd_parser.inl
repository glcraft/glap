#pragma once
#include "../cmd_parser.h"
#include "../utils.h"
#include "expected.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string_view>
namespace cmd
{
    auto Parser::parse(utils::Iterable<std::string_view> auto args) const -> result::PosExpected<result::Result> {
        result::Result result;
        result.program = args[0];

        auto itarg = std::next(args.begin());
        
        std::optional<std::reference_wrapper<const Command>> current_command = std::nullopt;
        // Find command
        // If no argument or starts with a flag, then it is a global command
        if (itarg == args.end() || std::string_view(*itarg).starts_with("-")) {
            current_command = this->get_global_command();
            if (!current_command.has_value()) {
                return result::make_unexpected(result::PositionnedError{
                        .error = result::Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = result::Error::Type::Command,
                        .code = result::Error::Code::NoGlobalCommand
                    },
                    .position = std::distance(args.begin(), itarg)
                });
            }
        } else {
            // Determine if the command name is a shortname (single character) or longname (multiple characters)
            auto char_len = ::utils::uni::utf8_char_length(*itarg);
            if (!char_len) 
                return result::make_unexpected(result::PositionnedError{
                        .error = result::Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = result::Error::Type::None,
                        .code = result::Error::Code::BadString
                    },
                    .position = std::distance(args.begin(), itarg)
                });
            auto found_cmd = commands.end();
            // If the command name is a single character, then search for a command with that shortname
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
            // If the command was not found, then return an error
            if (found_cmd != commands.end()) {
                current_command = std::ref(*found_cmd);
            } else {
                return result::make_unexpected(result::PositionnedError{
                    .error = result::Error{
                        .argument = *itarg,
                        .value = std::nullopt,
                        .type = result::Error::Type::Command,
                        .code = result::Error::Code::BadCommand
                    },
                    .position = std::distance(args.begin(), itarg)
                });
            }
            ++itarg;
        }
        // Parse command arguments
        auto parsed_cmd = parse_command(args.subspan(std::distance(args.begin(), itarg)), current_command.value());
        if (!parsed_cmd) {
            parsed_cmd.error().position += std::distance(args.begin(), itarg);
            return result::make_unexpected(parsed_cmd.error());
        }
        result.command = std::move(parsed_cmd.value());
        return std::move(result);
    }
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