#include "expected.h"
#include "fmt/format.h"
#include <algorithm>
#include <cmd_parser.h>
#include <functional>
#include <optional>
#include <string_view>

namespace cmd 
{
    namespace utils {
        result::Expected<uint8_t> utf8_char_length(std::string_view str) {
            auto c = str.front();
            if ((c & 0x80) == 0) {
                return 1;
            } else if ((c & 0xE0) == 0xC0) {
                return 2;
            } else if ((c & 0xF0) == 0xE0) {
                return 3;
            } else if ((c & 0xF8) == 0xF0) {
                return 4;
            } else {
                return unexpected<result::Error>(result::Error{
                    .argument = str,
                    .value = std::nullopt,
                    .type = result::Error::Type::None,
                    .code = result::Error::Code::BadString
                });
            }
        }
        result::Expected<size_t> utf8_length(std::string_view str) {
            uint8_t len = 0;
            for (auto itChar = str.begin(); itChar != str.end();) {
                auto c = *itChar;
                auto char_len = utf8_char_length(std::string_view{itChar, 1});
                if (!char_len) 
                    return unexpected<result::Error>(char_len.error());
                len += 1;
                itChar += char_len.value();
            }
            return len;
        }
        result::Expected<uint8_t> codepoint(std::string_view utf8) {
            char32_t codepoint = 0;
            auto it = utf8.begin();
            if ((*it & 0x80) == 0) {
                codepoint = *it;
            } else if ((*it & 0xE0) == 0xC0) {
                codepoint = (*it & 0x1F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else if ((*it & 0xF0) == 0xE0) {
                codepoint = (*it & 0x0F) << 12;
                codepoint |= (*(++it) & 0x3F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else if ((*it & 0xF8) == 0xF0) {
                codepoint = (*it & 0x07) << 18;
                codepoint |= (*(++it) & 0x3F) << 12;
                codepoint |= (*(++it) & 0x3F) << 6;
                codepoint |= *(++it) & 0x3F;
            } else {
                return unexpected<result::Error>(result::Error{
                    .argument = utf8,
                    .value = std::nullopt,
                    .type = result::Error::Type::None,
                    .code = result::Error::Code::BadString
                });
            }
            return codepoint;
        }
    }
    auto Parser::parse(std::span<std::string_view> args) const -> result::Expected<result::Result> {
        result::Result result;
        result.program = args[0];

        auto itarg = std::next(args.begin());
        std::optional<Command> current_command = std::nullopt;
        // Find command
        if (itarg->starts_with("-")) {
            current_command = this->get_global_command();
        } else {
            auto char_len = utils::utf8_char_length(*itarg);
            if (!char_len) 
                return unexpected<result::Error>(char_len.error());
            auto found_cmd = commands.end();
            if (char_len.value() <= itarg->size()) {
                auto codepoint = utils::codepoint(*itarg).value();
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
    auto Parser::parse_argument(std::span<std::string_view> args) const -> PosExpected<result::Argument> {
        
    }
    auto Parser::parse_flag(std::span<std::string_view> args) const -> PosExpected<result::Flag> {
        
    }
    auto Parser::parse_command(std::span<std::string_view> args, Command& command) const -> PosExpected<result::Command> {
        result::Command result_command;
        command.longname = command.longname;
        command.shortname = command.shortname;

        for (auto itarg = std::next(args.begin()); itarg != args.end(); itarg++) {
            auto name = *itarg;
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
            auto found_flag = std::find_if(
                this->global_command.begin(),
                result.flags.end(),
                [&name](const result::Flag& flag) {
                    return flag.name == name;
                }
            );
        }
    }
}