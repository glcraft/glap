#include "glap/model.h"
#ifdef GLAP_USE_MODULE
#ifndef GLAP_USE_STD_MODULE
import <concepts>;
import <variant>;
import <string_view>;
import <format>; // format exists on c++20 no need to fallback
import <iostream>;
#else
import std; // expected exists on c++23
#endif

import glap;

#ifdef GLAP_USE_FMT
import <fmt/format.h>;
namespace format = fmt;
#else
namespace format = std;
namespace format {
    template<typename... Args>
    constexpr auto print(std::format_string<Args...> format_string, Args&& ... args)
    {
        std::cout << std::format(format_string, std::forward<decltype(args)>(args)...);
    }
}
#endif
#else
#include <concepts>
#include <glap/glap.h>
#include <fmt/format.h>
#include <iterator>
#include <string_view>
#include <variant>
#include <iostream>

#include <glap/generators/style.h>
#include <glap/generators/help.h>
#endif


using flag_t = glap::model::Flag<
    glap::Names<"flag", 'f'>
>;
using verbose_t = glap::model::Flag<
    glap::Names<"verbose", 'v'>
>;
using flag_help_t = glap::model::Flag<
    glap::Names<"help", 'h'>
>;
using single_param_t = glap::model::Parameter<
    glap::Names<"single_param", 's'>
>;
using single_int_param_t = glap::model::Parameter<
    glap::Names<"to_int", glap::DISCARD>,
    [] (std::string_view v) -> glap::expected<int, glap::Discard> {
        try {
            return std::stoi(std::string(v));
        } catch(...) {
            /// In case stoi results in an error by exception,
            /// we return an error the parser can intercept
            return glap::make_unexpected(glap::DISCARD);
        }
    }
>;
using multi_param_t = glap::model::Parameters<
    glap::Names<"multi_param", 'm'>,
    glap::DISCARD // optional, = no limit
>;
using inputs_t = glap::model::Inputs<
    glap::DISCARD // optional, = no limit
>;

using command1_t = glap::model::Command<
    glap::Names<"command1", 'c'>, 
    flag_t, 
    verbose_t, 
    flag_help_t, 
    inputs_t
>;
using command2_t = glap::model::Command<
    glap::Names<"command2">, // notice that there is no short name here
    single_param_t, 
    multi_param_t, 
    flag_help_t, 
    inputs_t
>;

using program_t = glap::model::Program<"myprogram", glap::model::DefaultCommand<command1_t>, command2_t>;

using help_flag_t = glap::generators::help::Argument<
    "flag",
    glap::generators::help::Description<"test flag">
>;
using help_verbose_t = glap::generators::help::Argument<
    "verbose",
    glap::generators::help::Description<"test verbose flag">
>;
using help_single_param_t = glap::generators::help::Argument<
    "single_param",
    glap::generators::help::Description<"test single param">
>;
using help_multi_param_t = glap::generators::help::Argument<
    "multi_param",
    glap::generators::help::Description<"test multi param">
>;
using help_inputs_t = glap::generators::help::Argument<
    "INPUTS",
    glap::generators::help::Description<"test inputs">
>;
using help_command1_t = glap::generators::help::Command<
    "command1",
    glap::generators::help::FullDescription<"first command", "This is the first command defined in the program">,
    help_flag_t,
    help_verbose_t,
    help_single_param_t,
    help_multi_param_t,
    help_inputs_t
>;
using help_command2_t = glap::generators::help::Command<
    "command2",
    glap::generators::help::FullDescription<"second command", "This is the second command defined in the program">,
    help_single_param_t,
    single_int_param_t, 
    help_multi_param_t,
    help_inputs_t
>;

using help_program_t = glap::generators::help::Program<
    "myprogram",
    glap::generators::help::FullDescription<"myprogram", "This is my program">,
    help_command1_t,
    help_command2_t
>;

template <class T>
    requires requires { T::KIND; }
void print(const T& value) {
    if constexpr (requires { value.NAME; }) {
        fmt::print("  --{}: ", value.NAME);
    } else {
        fmt::print("  input: ");
    }
    if constexpr (requires { value.value; }) { // Value based
        fmt::print("\"{}\"", value.value.value());
    } else if constexpr (requires { value.values; }) { // Container based
        fmt::print("[ ");
        for (const auto& v : value.values) {
            fmt::print("\"{}\" ", v.value.value());
        }
        fmt::print("]");
    } else if constexpr (requires { value.occurences; }) { // Flag
        fmt::print("{}x", value.occurences);
    }
    fmt::print("\n");
}
template <class Names, class ...P>
auto print(const glap::model::Command<Names, P...>& command) {
    fmt::print("{}: \n", command.NAME);
    ([&] {
        print(std::get<P>(command.arguments));
    }(), ...);
}
template <auto Name, class ...C>
auto print(const glap::model::Program<Name, C...>& program) {
    fmt::print("{}\n", program.program);
    ([&] {
        using Command = typename glap::model::GetCommand<C>::Type;
        if (std::holds_alternative<Command>(program.command)) {
            const auto& command = std::get<Command>(program.command);
            if (command.template get_argument<"help">().occurences > 0) {
                // TODO: This is not working yet
                // fmt::print("Help:\n\n{}\n", glap::generators::get_help<help_program_t, Command>());
            } else {
                print(command);
            }
            return true;
        } else {
            return false;
        }
    }() || ...);
}

int main(int argc, char** argv)
{
    using namespace glap::model;
    using glap::DISCARD;
    auto args = std::vector<std::string_view>(argv, argv + argc);

    auto result = glap::parser<program_t>(args);

    if (result) {
        auto& v = *result;
        print(v);
    } else {
        fmt::print("{}\n\n", result.error().to_string());
        // fmt::print("Help:\n\n{}\n", glap::generators::get_help<help_program_t, program_t>());
        return 1;
    }
    return 0;
}
int main2(int argc, char** argv) {
    using namespace glap::generators;
    Style{
        .foreground_color = colors::foreground::RED,
        .bold = true,
        .underlined = true,
        .italic = true,
    }.apply();
    fmt::print("Hello, world!\n");
    Style{
        .foreground_color = colors::foreground::BLACK,
        .background_color = colors::background::GREEN,
        .bold = true,
        .underlined = true,
        .italic = true,
    }.apply();
    fmt::print("Hello, world!\n");
    Style::reset();
    fmt::print("Hello, world!\n");
    return 0;
}