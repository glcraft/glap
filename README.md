# glap : gly's command line parser

A UTF-8 command line parser written in C++20 respecting conventionals shell arguments.

## Notable features

### Up to date library

The library is written in C++20 with language features like concepts, ranges, views... which make the library easier to use and maintain.

### Low heap memory usage

Every string used by the library is a view. Take a look [here](#something-about-lifetime) for more details.

### Error analysis and validators

Several error types is catch and returned for a better analysis or the arguments. Validators over input and argument values can be set up.

Moreover, the library doesn't use exception but expected class in return instead.

### UTF-8 compliant

If you want to use emoji or non latin alphabet in long or short name, you can !

## Something about lifetime...

Except for dynamic array used to declare arguments and flags, the library doesn't own strings. Every string used in the library is a view from arguments or strings in the code. That means even config objects doesn't own strings !

Take a look at this :

```cpp
glap::Parser parser;
// "read" is a static const strings, the command owns a view of it
auto& cmd_compress = parser.make_command("read", 'r'); 
// You cannot declare a command using a string instance directly
// The following line will be UB during parsing/reading result since the string doesn't exist after that line
auto& cmd_compress = parser.make_command(std::string("read"), 'r'); 
// if you want to use a string, store it in a variable and keep it until you finish with parsing result :
auto str = std::string("read")
auto& cmd_compress = parser.make_command(str, 'r'); // = OK
```

Since the program arguments have the same lifetime of the program execution, you don't have to worry about their lifetime.

## Usage 

```cpp
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
```

```
> app read --output=file.log file1.txt file2.txt
program name: glap-test
command: read
argument(s):
  argument output = file.log
  input: file1.txt
  input: file2.txt

> app --output=file.log file1.txt file2.txt
program name: glap-test
command: read
argument(s):
  argument output = file.log
  input: file1.txt
  input: file2.txt

> app --output=file.log file.zip
error parsing arguments: at argument 2: "file.zip" (type: input) : invalid value

> app file1.txt file2.txt
error parsing arguments: at argument 1: "output" (type: argument) : required parameter(s) missing
```