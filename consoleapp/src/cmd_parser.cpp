#include <cmd_parser.h>

namespace cmd 
{
    auto Parser::parse(std::span<std::string_view> args) const -> Expected<result::Result> {

    }
    auto Parser::parse_argument(std::span<std::string_view> args) const -> PosExpected<result::Argument> {
        
    }
    auto Parser::parse_flag(std::span<std::string_view> args) const -> PosExpected<result::Flag> {
        
    }
    auto Parser::parse_command(std::span<std::string_view> args) const -> PosExpected<result::Command> {
        
    }
}