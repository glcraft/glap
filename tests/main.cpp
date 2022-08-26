#include "glap/v2/parser.h"
#include <ranges>
#include <charconv>
#include <fmt/format.h>
#include <variant>

struct Param1 {
    static constexpr std::string_view longname = "param1";  
    static constexpr std::optional<char32_t> shortname = 'p';
    static constexpr auto type = glap::v2::ParameterType::Argument;
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

template <class ...P>
struct Print 
{};

template <class Names, auto... T>
struct Print<glap::v2::Argument<Names, T...>> {
    using value_type= glap::v2::Argument<Names, T...>;
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
struct Print<glap::v2::Flag<P...>> {
    using value_type= glap::v2::Flag<P...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: {}x\n", v.Longname, v.occurences);
    }
};
template <auto ...Args>
struct Print<glap::v2::Inputs<Args...>> {
    using value_type= glap::v2::Inputs<Args...>;
    void operator()(const value_type& v) const {
        fmt::print("    inputs: ");
        auto nb=0;
        for (auto& val : v.values)
            fmt::print("{}\"{}\"", nb++>0 ? ", " : "", val.value.value_or("none"));
        fmt::print("\n ");
    }
};
template <class Names, auto N, auto ...Args>
struct Print<glap::v2::Arguments<Names, N, Args...>> {
    using value_type= glap::v2::Arguments<Names, N  , Args...>;
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
auto print_command(glap::v2::Command<Names, P...>& command) {
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
    using namespace glap::v2;
    // Parser<
    //     glap::v2::Command<glap::v2::Names<"commands", glap::v2::discard>, Param1>,
    //     glap::v2::Command<glap::v2::Names<"command", 'c'>, 
    //         glap::v2::Flag<glap::v2::Names<"flag", 'f'>>,
    //         glap::v2::Arguments<glap::v2::Names<"flags", 'a'>, 4>,
    //         glap::v2::Argument<glap::v2::Names<"integer", 'g'>, to_int>
    //         glap::v2::Inputs<>
    // >> parser;
    Parser<glap::v2::Command<glap::v2::Names<"othercommand", 't'>, glap::v2::Flag<glap::v2::Names<"flag", 'f'>>>,
        glap::v2::Command<glap::v2::Names<"command", glap::v2::discard>, 
            glap::v2::Flag<glap::v2::Names<"flag", 'f'>>,
            glap::v2::Argument<glap::v2::Names<"arg", 'a'>, discard, is_hello_world>,
            glap::v2::Arguments<glap::v2::Names<"args", 'b'>>,
            Inputs<>
        >
    > parser;
    
    auto result = parser.parse(std::span{argv, argv+argc} | std::views::transform([](auto arg) {return std::string_view{arg};}) );

    if (result) {
        auto& v = *result;
        fmt::print("program: {}\n", v.program);
        std::visit([](auto& command){ print_command (command); }, v.command);
    } else {
        fmt::print("{}\n", result.error().to_string());
        return 1;
    }
    return 0;
}