#include <glap/glap.h>
#include <ranges>
#include <charconv>
#include <fmt/format.h>
#include <type_traits>
#include <variant>

struct Param1 {
    static constexpr std::string_view longname = "param1";  
    static constexpr std::optional<char32_t> shortname = 'p';
    static constexpr auto type = glap::model::ArgumentType::Parameter;
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
struct Print<glap::model::Parameter<Names, T...>> {
    using value_type= glap::model::Parameter<Names, T...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: ", v.longname);
        if (v.value) {
            fmt::print("\"{}\"\n", v.value.value());
        } else {
            fmt::print("none\n");
        }
    }
};
template <class ...P>
struct Print<glap::model::Flag<P...>> {
    using value_type= glap::model::Flag<P...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: {}x\n", v.longname, v.occurences);
    }
};
template <class T> 
    requires requires (T a){
        a.values;
        T::type == glap::model::ArgumentType::Input;
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
struct Print<glap::model::Parameters<Names, N, Args...>> {
    using value_type= glap::model::Parameters<Names, N  , Args...>;
    void operator()(const value_type& v) const {
        fmt::print("    --{}: ", v.longname);
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
auto print_command(glap::model::Command<Names, P...>& command) {
    fmt::print("command: {}\n", command.longname);
    ([&] {
        print<P>(std::get<P>(command.params));
    }(), ...);
    
}
bool is_hello_world(std::string_view v) {
    return v == "hello" || v == "world";
}

int main(int argc, char** argv)
{
    using namespace glap::model;
    using glap::discard;

    using ParserCommand = Command<glap::Names<"command", 'c'>, 
        Flag<glap::Names<"flag", 'f'>>,
        Parameter<glap::Names<"arg", 'a'>, discard, is_hello_world>,
        Parameters<glap::Names<"args", 'b'>>,
        Inputs<>
    >;
    glap::Parser<"glap", glap::DefaultCommand::FirstDefined, Command<glap::Names<"othercommand", 't'>, Flag<glap::Names<"flag", 'f'>>>,
        ParserCommand
    > parser;


    using HelpCommand = glap::help::model::Command<"command", glap::help::model::Description<"first defined command">,
        glap::help::model::Argument<"flag", glap::help::model::Description<"flag example">>,
        glap::help::model::Argument<"arg", glap::help::model::Description<"single parameter example">>,
        glap::help::model::Argument<"args", glap::help::model::Description<"multiple parameters example">>,
        glap::help::model::Argument<"INPUTS", glap::help::model::Description<"inputs description">>
    >;

    using HelpProgram = glap::help::model::Program<"glap-example",
        glap::help::model::FullDescription<"example program", "This is an exemple of the program description">, 
        HelpCommand
    >;
    {
        auto help_str = glap::get_help<HelpProgram, decltype(parser)>();
        fmt::print("# Help Program :\n{}\n\n", help_str);
    }
    {
        auto help_str = glap::get_help<HelpCommand, ParserCommand>();
        fmt::print("# Help Command \"command\" :\n{}\n\n", help_str);
    }
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