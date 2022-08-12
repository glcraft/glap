#include <cmd_parser.h>
#include <fmt/format.h>
#include <span>
#include <tl/expected.hpp>
#include <string_view>
#include <ranges>

auto parse_argumnts(int argc, char **argv) -> tl::expected<cmd::result::Result, cmd::result::Error> 
{
    cmd::Parser parser;
    parser.make_command("compress", 'c').set_description("Compress files and directories");
    parser.make_command("extract", 'x').set_description("Extract files from compressed file");
    parser.make_command("list", 'l').set_description("Explore compressed file");

    return parser.parse(std::span(argv, argv+argc));
}

int main(int argc, char** argv)
{
    auto arg_result = parse_argumnts(argc, argv);
    if (!arg_result) {
        fmt::print("{}\n", arg_result.error().to_string());
        return 1;
    } 
    auto arguments = std::move(*arg_result);
    fmt::print("command: {}\n", arguments.command.name);
    return 0;
}