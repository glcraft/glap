#include <cmd_parser.h>
#include <fmt/format.h>
#include <span>
#include <tl/expected.hpp>
#include <string_view>
#include <ranges>
#include <variant>

auto parse_argumnts(int argc, char **argv) -> cmd::result::PosExpected<cmd::result::Result> 
{
    cmd::Parser parser;
    auto& cmd_compress = parser.make_command("compress", 'c').set_description("Compress files and directories");
    cmd_compress.make_argument("output", 'o').set_description("Output file").set_required(true);
    cmd_compress
        .make_argument("check", 'c')
        .set_validator([](std::string_view value) -> bool {
            using namespace std::string_view_literals;
            static auto constexpr types = std::array{"hello"sv, "world"sv};
            return std::find(types.begin(), types.end(), value) != types.end();
        })
        .set_description("Output file");
    cmd_compress.make_flag("verbose", 'v').set_description("Verbose mode").set_max(3);
    cmd_compress.make_flag("Flag1", 'a').set_description("Test flag 1");
    cmd_compress.make_flag("Flag2", 'b').set_description("Test flag 2");
    parser.set_global_command("compress");
    parser.make_command("extract", 'x').set_description("Extract files from compressed file");
    parser.make_command("list", 'l').set_description("Explore compressed file");

    auto result = parser.parse(std::span(argv, argv+argc));
    return std::move(result);
}

int main(int argc, char** argv)
{
    auto arg_result = parse_argumnts(argc, argv);
    if (!arg_result) {
        fmt::print("error parsing arguments: {}\n", arg_result.error().to_string());
        return 1;
    } 
    auto arguments = std::move(arg_result.value());

    fmt::print("program name: {}\n", arguments.program);
    fmt::print("command: {}\n", arguments.command.name);
    if (!arguments.command.parameters.empty()) {
        fmt::print("argument(s):\n");
        for (auto const& parameter : arguments.command.parameters) {
            if (std::holds_alternative<cmd::result::Flag>(parameter)) {
                auto const& flag = std::get<cmd::result::Flag>(parameter);
                fmt::print("  flag {} ({}x)\n", flag.name, flag.occurrence);
            } else {
                const auto& argument = std::get<cmd::result::Argument>(parameter);
                fmt::print("  argument {} = {}\n", argument.name, argument.value);
            }
        }
    }
    return 0;
}