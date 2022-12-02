#include "glap/parser.h"
#include <glap_def.h>
#include <string_view>
#include <sys/errno.h>
#include <yaml-cpp/yaml.h>
#include <span>
#include <iostream>
#include <ranges>
#include <fmt/format.h>
#include <fmt/os.h>

auto emit_error(std::string_view msg, auto... args) {
    fmt::print(stderr, "error: ", fmt::format(fmt::runtime(msg), args...));
    exit(1);
}

auto generate_header(YAML::Node config, fmt::ostream output) {
    output.print("#pragma once");
}

auto parse_yaml(std::string_view path) -> YAML::Node {
    auto yaml_path = std::string{path};
    try {
        return YAML::LoadFile(yaml_path);
    } catch (YAML::BadFile &e) {
        emit_error("failed to open file '{}'", yaml_path);
    } catch (YAML::ParserException &e) {
        emit_error("failed to parse file '{}'\n{}", yaml_path, e.what());
    }
    return {};
}

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
        emit_error("yaml path is not set");
    }
    auto yaml_path = std::string{command.get_argument<"yaml">().value.value()};
    auto config = parse_yaml(yaml_path);

    auto type = command.get_argument<"type">().value.value_or("header");
    auto output_path = std::string{command.get_argument<"output">().value.value_or("output.h")};
    auto output = fmt::output_file(output_path.c_str());
    
    if (errno != 0) {
        auto error_msg = std::strerror(errno);
        emit_error("failed to write in file '{}': {}", output_path, error_msg);
    }

    if (type == "header") {
        generate_header(std::move(config), std::move(output));
    }

    return 0;
    // do something with program
}