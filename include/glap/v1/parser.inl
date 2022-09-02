#pragma once
#include "parser.h"
#include "../common/utils.h"
#include "../common/expected.h"
#include "../common/error.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <string_view>
#include <variant>
#include <unordered_set>
namespace glap::v1
{
    auto Parser::parse(utils::Iterable<std::string_view> auto args) const -> PosExpected<result::Result> {
        result::Result result;
        result.program = args[0];

        auto itarg = std::next(args.begin());
        
        std::optional<std::reference_wrapper<const config::Command>> current_command = std::nullopt;
        // Find command
        // If no argument or starts with a flag, then it is a global command
        if (itarg == args.end() || std::string_view(*itarg).starts_with("-")) {
            current_command = this->get_global_command();
        } else {
            // Determine if the command name is a shortname (single character) or longname (multiple characters)
            auto char_len = glap::utils::uni::utf8_char_length(*itarg);
            if (!char_len) 
                return make_unexpected(PositionnedError{
                        .error = Error{
                        .argument = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::BadString
                    },
                    .position = std::distance(args.begin(), itarg)
                });
            auto found_cmd = commands.end();
            // If the command name is a single character, then search for a command with that shortname
            if (char_len.value() <= std::string_view(*itarg).size()) {
                auto codepoint = glap::utils::uni::codepoint(*itarg).value();
                found_cmd = std::find_if(commands.begin(), commands.end(), [codepoint](const config::Command& command) {
                    return command.shortname == codepoint;
                });
            } else {
                found_cmd = std::find_if(commands.begin(), commands.end(), [&](const config::Command& command) {
                    return command.longname == *itarg;
                });
            }
            // If the command was not found, then return an error
            if (found_cmd != commands.end()) {
                current_command = std::ref(*found_cmd);
                ++itarg;
            } else {
                current_command = this->get_global_command();
            }
        }
        if (!current_command.has_value()) {
            return make_unexpected(PositionnedError{
                .error = Error{
                    .argument = "",
                    .value = std::nullopt,
                    .type = Error::Type::Command,
                    .code = Error::Code::NoGlobalCommand
                },
                .position = std::distance(args.begin(), itarg)
            });
        }

        // Parse command arguments
        auto parsed_cmd = parse_command(args.subspan(std::distance(args.begin(), itarg)), current_command.value());
        if (!parsed_cmd) {
            parsed_cmd.error().position += std::distance(args.begin(), itarg);
            return make_unexpected(parsed_cmd.error());
        }
        result.command = std::move(parsed_cmd.value());
        return std::move(result);
    }
    template <utils::Iterator<std::string_view> Iter>
    auto Parser::parse_long_argument(Iter& itarg, Iter end, const config::Command& command, result::Command& result_command) const -> PosExpected<bool>
    {
        std::string_view arg = *itarg;
        std::string_view name;
        std::optional<std::string_view> value;
        // argument format: --name=value
        auto equal_pos = arg.find('=');
        if (equal_pos == std::string_view::npos) {
            //is a flag
            name = arg.substr(2);
            value = std::nullopt;
        } else {
            //is an argument
            value = arg.substr(equal_pos + 1);
            name = arg.substr(2, equal_pos - 2);
        }
        auto long_finder = [name](const auto& param) {
            return param.longname == name;
        };
        auto found_flag = std::find_if(command.flags.begin(), command.flags.end(), long_finder);
        if (found_flag != command.flags.end()) {
            if (value) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = name,
                        .value = value,
                        .type = Error::Type::Flag,
                        .code = Error::Code::FlagWithValue
                    },
                    .position = 0
                });
            }
            auto res = this->add_flag(result_command, *found_flag, name);
            if (!res) {
                return res;
            }
        } else {
            auto found_argument = std::find_if(command.arguments.begin(), command.arguments.end(), long_finder);
            if (found_argument != command.arguments.end()) {
                if (!value) {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .argument = name,
                            .value = value,
                            .type = Error::Type::Argument,
                            .code = Error::Code::MissingValue
                        },
                        .position = 0
                    });
                }
                auto res = this->add_argument(result_command, *found_argument, name, value.value());
                if (!res) {
                    return res;
                }
            } else {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = name,
                        .value = value,
                        .type = Error::Type::Argument,
                        .code = Error::Code::UnknownParameter
                    },
                    .position = 0
                });
            }
        }
        return true;
    }
    template <utils::Iterator<std::string_view> Iter>
    auto Parser::parse_short_argument(Iter& itarg, Iter endarg, const config::Command& command, result::Command& result_command) const -> PosExpected<bool>
    {
        std::string_view arg = *itarg;
        std::string_view name;
        std::optional<std::string_view> value;
        // argument format: -n [value] or -fff
        name = arg.substr(1);
        if (name.size() > glap::utils::uni::utf8_char_length(name).value()) {
            //multi flags
            for (auto iName = 0; iName<name.size();) {
                auto exp_len = glap::utils::uni::utf8_char_length(name.substr(iName));
                if (!exp_len)
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .argument = name,
                            .value = value,
                            .type = Error::Type::Flag,
                            .code = Error::Code::BadString
                        },
                        .position = 0
                    });
                auto len = exp_len.value();
                auto exp_codepoint = glap::utils::uni::codepoint(name.substr(iName, len));
                if (!exp_codepoint)
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .argument = name,
                            .value = std::nullopt,
                            .type = Error::Type::Flag,
                            .code = Error::Code::BadString
                        },
                        .position = 0
                    });
                auto codepoint = exp_codepoint.value();
                auto found_flag = std::find_if(command.flags.begin(), command.flags.end(), [codepoint](const auto& flag) {
                    return flag.shortname == codepoint;
                });
                if (found_flag == command.flags.end()) {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .argument = name,
                            .value = std::nullopt,
                            .type = Error::Type::Flag,
                            .code = Error::Code::UnknownParameter
                        },
                        .position = 0
                    });
                }
                auto res = this->add_flag(result_command, *found_flag, name);
                if (!res) {
                    return res;
                }
                iName += len;
            }
        } else {
            // one flag or argument
            name = arg.substr(1);
            auto exp_codepoint = glap::utils::uni::codepoint(name);
            if (!exp_codepoint)
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .argument = name,
                        .value = std::nullopt,
                        .type = Error::Type::Flag,
                        .code = Error::Code::BadString
                    },
                    .position = 0
                });
            auto codepoint = exp_codepoint.value();
            auto found_flag = std::find_if(command.flags.begin(), command.flags.end(), [codepoint](const auto& flag) {
                return flag.shortname == codepoint;
            });
            if (found_flag != command.flags.end()) {
                auto res = this->add_flag(result_command, *found_flag, name);
                if (!res) {
                    return res;
                }
            } else {
                auto found_argument = std::find_if(command.arguments.begin(), command.arguments.end(), [codepoint](const auto& argument) {
                    return argument.shortname == codepoint;
                });
                if (found_argument != command.arguments.end()) {
                    auto itvalue = ++itarg;
                    if (itvalue == endarg) {
                        return make_unexpected(PositionnedError{
                            .error = Error{
                                .argument = name,
                                .value = std::nullopt,
                                .type = Error::Type::Argument,
                                .code = Error::Code::MissingValue
                            },
                            .position = 0
                        });
                    }
                    auto res = this->add_argument(result_command, *found_argument, name, *itvalue);
                    if (!res) {
                        return res;
                    }
                } else {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .argument = name,
                            .value = std::nullopt,
                            .type = Error::Type::Argument,
                            .code = Error::Code::UnknownParameter
                        },
                        .position = 0
                    });
                }
            }
        }
        
        return true;
    }

    

    auto Parser::parse_command(utils::Iterable<std::string_view> auto args, const config::Command& command) const -> PosExpected<result::Command> {
        result::Command result_command;
        result_command.name = command.longname;

        for (auto itarg = args.begin(); itarg != args.end(); itarg++) {
            auto name = std::string_view(*itarg);
            std::optional<std::string_view> value = std::nullopt;
            bool is_short = false;
            if (name.starts_with("---")) {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        *itarg,
                        std::nullopt,
                        Error::Type::None,
                        Error::Code::SyntaxError
                    },
                    .position = std::distance(args.begin(), itarg)
                });
            }
            else if (name == "--") {
                break;
            }
            else if (name == "--help") {
                //no parameter for help
                result_command.parameters.clear();
                result_command.help = true;
                break;
            }
            PosExpected<bool> parameter;
            if (name.starts_with("--")) {
                // argument format: --name=value
                parameter = this->parse_long_argument(itarg, args.end(), command, result_command);
            } else if (name.starts_with("-")) {
                parameter = this->parse_short_argument(itarg, args.end(), command, result_command);
            } else {
                //TODO: is an input
                parameter = this->add_input(result_command, command, name);
            }
            if (!parameter) {
                parameter.error().position += std::distance(args.begin(), itarg);
                return make_unexpected(parameter.error());
            }
        }
        //post validation
        std::unordered_set<std::string_view> required_arguments;
        for (auto& argument : command.arguments) {
            if (argument.required) {
                required_arguments.insert(argument.longname);
            }
        }
        for (auto param : result_command.parameters) {
            if (std::holds_alternative<result::Argument>(param)) {
                auto argument = std::get<result::Argument>(param);
                if (auto found = required_arguments.find(argument.name); found != required_arguments.end()) {
                    required_arguments.erase(found);
                }
            }
        }
        if (!required_arguments.empty()) {
            return make_unexpected(PositionnedError{
                .error = Error{
                    .argument = *required_arguments.begin(),
                    .value = std::nullopt,
                    .type = Error::Type::Argument,
                    .code = Error::Code::RequiredArgument
                },
                .position = 0
            });
        }
        return std::move(result_command);
    }
}