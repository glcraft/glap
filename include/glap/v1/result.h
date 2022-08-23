#pragma once
#include <string_view>
#include <span>
#include <variant>
#include "config.h"
#include "../common/utils.h"
#include "../common/fmt.h"

namespace glap::v1::result
{
    using Input = std::string_view;
    struct Argument {
        std::string_view name;
        std::string_view value;
        const glap::v1::config::Argument& argument_parser;
    };
    struct Flag {
        std::string_view name;
        uint32_t occurrence;
        const glap::v1::config::Flag& flag_parser;
    };
    using Parameter = std::variant<Argument, Flag, Input>;
    struct Command {
        std::string_view name;
        std::vector<Parameter> parameters;
        bool help;

        Flag& add_flag(const glap::v1::config::Flag& flag) {
            auto found = std::find_if(parameters.begin(), parameters.end(), [name=flag.longname](const auto& parameter) {
                return std::holds_alternative<Flag>(parameter) && std::get<Flag>(parameter).name == name;
            });
            if (found == parameters.end()) {
                parameters.push_back(Flag{flag.longname, 1, flag});
                return std::get<Flag>(parameters.back());
            } else {
                std::get<Flag>(*found).occurrence++;
                return std::get<Flag>(*found);
            }
        }
    };
    struct Result {
        std::string_view program;
        Command command;
        // std::vector<Parameter> parameters;
    };
    
}