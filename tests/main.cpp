#include <glap.h>
#include <fmt/format.h>
#include <string_view>
#include <array>

int main(int argc, char** argv)
{
    glap::Parser parser;
    // Add a command
    auto& cmd_compress = parser.make_command("read", 'r').set_description("Compress files and directories");
    // Setup an input validator
    cmd_compress.set_input_validator([](std::string_view input) -> bool {
        //Only accept *.txt files
        return input.ends_with(".txt");
    });
    // Add an "output" argument. Set it required.
    cmd_compress.make_argument("output", 'o').set_description("Output file").set_required(true);
    // Add a "chack" argument with a custom validator
    cmd_compress
        .make_argument("mode", 'm')
        .set_description("Type of read")
        .set_validator([](std::string_view value) -> bool {
            // only accept "debug" or "release"
            using namespace std::string_view_literals;
            static auto constexpr types = std::array{"debug"sv, "release"sv};
            return std::find(types.begin(), types.end(), value) != types.end();
        });
    // Add a simple flag
    cmd_compress.make_flag("flag1", 'a').set_description("Test flag 1");
    // Add a simple flag, without short name
    cmd_compress.make_flag("flag2").set_description("Test flag 2");
    // Add "verbose" flag, usable up to 3x (like -vvv)
    cmd_compress.make_flag("verbose", 'v').set_description("Verbose mode").set_max(3);
    // Setup the global command to "compress" defined earlier
    parser.set_global_command("read");

    //Parse program arguments
    auto arg_result = parser.parse(std::span(argv, argv+argc));
    
    // if there is a parsing error
    if (!arg_result) {
        // ...display it
        fmt::print("error parsing arguments: {}\n", arg_result.error().to_string());
        return 1;
    } 
    auto arguments = std::move(arg_result.value());

    //Just print what we receive
    fmt::print("program name: {}\n", arguments.program);
    fmt::print("command: {}\n", arguments.command.name);
    if (!arguments.command.parameters.empty()) {
        fmt::print("argument(s):\n");
        for (auto const& parameter : arguments.command.parameters) {
            if (std::holds_alternative<glap::result::Flag>(parameter)) {
                auto const& flag = std::get<glap::result::Flag>(parameter);
                fmt::print("  flag {} ({}x)\n", flag.name, flag.occurrence);
            } else if (std::holds_alternative<glap::result::Argument>(parameter)) {
                const auto& argument = std::get<glap::result::Argument>(parameter);
                fmt::print("  argument {} = {}\n", argument.name, argument.value);
            } else {
                fmt::print("  input: {}\n", std::get<glap::result::Input>(parameter));
            }
        }
    }
}