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
#include <string_view>
#include <variant>

#ifdef GLAP_USE_FMT
#include <fmt/format.h>
using format = fmt;
#else
#include <format>
#include <iostream>
using format = std;
namespace format {
    template<typename... Args>
    constexpr auto print(std::format_string<Args...> format_string, Args&& ... args)
    {
        std::cout << std::format(format_string, std::forward<decltype(args)>(args)...);
    }
}
#endif
#endif

template <class T>
    requires requires { T::type; }
void print(const T& value) {
    if constexpr (requires { value.longname; }) {
        format::print("  --{}: ", value.longname);
    } else {
        format::print("  input: ");
    }
    if constexpr (requires { value.value; }) { // Value based
        format::print("\"{}\"", value.value.value());
    } else if constexpr (requires { value.values; }) { // Container based
        format::print("[ ");
        for (const auto& v : value.values) {
            format::print("\"{}\" ", v.value.value());
        }
        format::print("]");
    } else if constexpr (requires { value.occurences; }) { // Flag
        format::print("{}x", value.occurences);
    }
    format::print("\n");
}
template <class Names, class ...P>
auto print(const glap::model::Command<Names, P...>& command) {
    format::print("{}: \n", command.longname);
    ([&] {
        print(std::get<P>(command.arguments));
    }(), ...);
}
template <auto Name, auto D, class ...C>
auto print(const glap::model::Program<Name, D, C...>& program) {
    format::print("{}\n", program.program);
    ([&] {
        if (std::holds_alternative<C>(program.command)) {
            print(std::get<C>(program.command));
            return true;
        } else {
            return false;
        }
    }() || ...);
}

using flag_t = glap::model::Flag<
    glap::Names<"flag", 'f'>
>;
using verbose_t = glap::model::Flag<
    glap::Names<"verbose", 'v'>
>;
using help_t = glap::model::Flag<
    glap::Names<"help", 'h'>
>;
using single_param_t = glap::model::Parameter<
    glap::Names<"single_param", 's'>
>;
using single_int_param_t = glap::model::Parameter<
    glap::Names<"to_int", glap::discard>,
    [] (std::string_view v) -> glap::expected<int, glap::Discard> {
        try {
            return std::stoi(std::string(v));
        } catch(...) {
            /// In case stoi results in an error by exception,
            /// we return an error the parser can intercept
            return glap::make_unexpected(glap::discard);
        }
    }
>;
using multi_param_t = glap::model::Parameters<
    glap::Names<"multi_param", 'm'>,
    glap::discard // optional, = no limit
>;
using inputs_t = glap::model::Inputs<
    glap::discard // optional, = no limit
>;

using command1_t = glap::model::Command<
    glap::Names<"command1", 'c'>,
    flag_t,
    verbose_t,
    help_t,
    inputs_t
>;
using command2_t = glap::model::Command<
    glap::Names<"command2">, // notice that there is no short name here
    single_param_t,
    single_int_param_t,
    multi_param_t,
    help_t,
    inputs_t
>;

using program_t = glap::model::Program<"myprogram", glap::model::DefaultCommand::FirstDefined, command1_t, command2_t>;

int main(int argc, char** argv)
{
    using namespace glap::model;
    using glap::discard;
    auto args = std::vector<std::string_view>(argv, argv + argc);

    auto result = glap::parser<program_t>(args);

    if (result) {
        auto& v = *result;
        print(v);
    } else {
        format::print("{}\n", result.error().to_string());
        return 1;
    }
    return 0;
}