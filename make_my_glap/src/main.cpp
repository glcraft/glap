#include "glap/parser.h"
#include <glap_def.h>
#include <span>
#include <iostream>
#include <ranges>



int main(int argc, char** argv) {
    // store arguments in a vector of string_view
    std::vector<std::string_view> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    auto args_result = glap::parser<args::program_t>(args);
    if (!args_result) {
        std::cerr << args_result.error().to_string() << std::endl;
        return 1;
    }
    auto program = args_result.value();
    auto &command = std::get<args::command_generate_t>(program.command);
    if (!command.get_argument<"yaml">().value) {
        std::cout << "yaml path is not set" << std::endl;
    }
    return 0;
    // do something with program
}