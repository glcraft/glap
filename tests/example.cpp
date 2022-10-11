#include "glap/common/error.h"
#include "glap/v2/model.h"
#include "glap/v2/parser.h"
#include "glap/common/utils.h"
#include "glap/v2/help.h"
#include <ranges>
#include <charconv>
#include <fmt/format.h>
#include <type_traits>
#include <variant>

struct Param1 {
    static constexpr std::string_view longname = "param1";  
    static constexpr std::optional<char32_t> shortname = 'p';
    static constexpr auto type = glap::v2::model::ParameterType::Argument;
    using result_type = void;
};

auto to_int(std::string_view str) -> tl::expected<int, std::errc> {
    int result;
    auto [_, ec] { std::from_chars(str.data(), str.data() + str.size(), result) };
 
    if (ec == std::errc()) {
        return result;
    } else {
        return tl::make_unexpected(ec);
    }
}

template <class P>
struct Print 
{};

template <class Names, auto... T>
struct Print<glap::v2::model::Argument<Names, T...>> {
    using value_type= glap::v2::model::Argument<Names, T...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: ", v.Longname);
        if (v.value) {
            fmt::print("\"{}\"\n", v.value.value());
        } else {
            fmt::print("none\n");
        }
    }
};
template <class ...P>
struct Print<glap::v2::model::Flag<P...>> {
    using value_type= glap::v2::model::Flag<P...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: {}x\n", v.Longname, v.occurences);
    }
};
template <class T> 
    requires requires (T a){
        a.values;
        T::type == glap::v2::model::ParameterType::Input;
    }
struct Print<T> {
    using value_type= T;
    void operator()(const value_type& v) const {
        fmt::print("    inputs: ");
        auto nb=0;
        for (auto& val : v.values)
            fmt::print("{}\"{}\"", nb++>0 ? ", " : "", val.value.value_or("none"));
        fmt::print("\n ");
    }
};
template <class Names, auto N, auto ...Args>
struct Print<glap::v2::model::Arguments<Names, N, Args...>> {
    using value_type= glap::v2::model::Arguments<Names, N  , Args...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: ", v.Longname);
        auto nb=0;
        if (v.values.empty()) {
            fmt::print("none\n");
            return;
        }
        for (auto& val : v.values)
            fmt::print("{}\"{}\"", nb++>0 ? ", " : "", val.value.value_or("none"));
        fmt::print("\n");
    }
};
template <class T>
static constexpr auto print = Print<T>{};

template <class Names, class ...P>
auto print_command(glap::v2::model::Command<Names, P...>& command) {
    fmt::print("command: {}\n", command.Longname);
    ([&] {
        print<P>(std::get<P>(command.params));
    }(), ...);
    
}
bool is_hello_world(std::string_view v) {
    return v == "hello" || v == "world";
}

int main(int argc, char** argv)
{
    using namespace glap::v2::model;
    using glap::v2::discard;
    glap::v2::Parser<glap::v2::DefaultCommand::FirstDefined, Command<glap::v2::Names<"othercommand", 't'>, Flag<glap::v2::Names<"flag", 'f'>>>,
        Command<glap::v2::Names<"command", 'c'>, 
            Flag<glap::v2::Names<"flag", 'f'>>,
            Argument<glap::v2::Names<"arg", 'a'>, discard, is_hello_world>,
            Arguments<glap::v2::Names<"args", 'b'>>,
            Inputs<>
        >
    > parser;

    using help = glap::v2::help::model::Program<"glap-example",
        glap::v2::help::model::FullDescription<"example program", "This is an exemple of the program description">, 
        glap::v2::help::model::Command<"othercommand", glap::v2::help::model::Description<"first defined command">>
        // glap::v2::help::model::Command<"command", glap::v2::help::model::Description<"More complete command">>
    >;

    auto help_str = glap::v2::get_help<help, decltype(parser)>();
    fmt::print("{}\n", help_str);
    // auto result = parser.parse(std::span{argv, argv+argc} | std::views::transform([](auto arg) {return std::string_view{arg};}) );

    // if (result) {
    //     auto& v = *result;
    //     fmt::print("program: {}\n", v.program);
    //     std::visit([](auto& command){ print_command (command); }, v.command);
    // } else {
    //     fmt::print("{}\n", result.error().to_string());
    //     return 1;
    // }
    return 0;
}