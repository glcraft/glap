#include <argumentum/argparse.h>
#include <fmt/format.h>
#include "config.h"
#include <fmt/core.h>
#include <tl/expected.hpp>
#include <functional>
#include <iterator>
#include <string_view>
#include <concepts>
#include <array>
#include <iostream>
#include <memory>

auto parse_argumnts(int argc, char **argv) -> tl::expected<argumentum::ParseResult, std::string> 
{
    auto parser = argumentum::argument_parser{};
    auto params = parser.params();
    parser.config().program( argv[0] ).description( "Archive Explorer" );
    params.add_command<CompressConfig>("compress").help( "Compress files" );
    params.add_command<ListConfig>("list").help( "List files" );

    auto result = parser.parse_args(argc, argv);
    if (!result)
    {
        std::string buffer;
        buffer.reserve(512);
        auto it = std::back_inserter(buffer);
        it = fmt::format_to(it, "Unable to parse arguments\n");
        for (const auto& err : result.errors) {
            it = fmt::format_to(it, "option \"{}\": error {}\n", err.option, err.errorCode);
        }
        return tl::make_unexpected(buffer);
    }
    return std::move(result);
}

int main(int argc, char** argv)
{
    auto arg_result = parse_argumnts(argc, argv);
    if (!arg_result) {
        fmt::print("{}\n", arg_result.error());
        return 1;
    } 
    auto arguments = std::move(*arg_result);
    if (arguments.commands.empty()) {
        fmt::print("No command specified\n");
        return 1;
    }
    fmt::print("command: {}\n", arguments.commands.back()->getName());
    return 0;
}