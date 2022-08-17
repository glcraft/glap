#include "glapv2/parser.h"
#include "tl/expected.hpp"
#include <glap.h>
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <array>
#include <charconv>

struct Param1 {
    static constexpr std::string_view longname = "param1";  
    static constexpr std::optional<char32_t> shortname = 'p';
    static constexpr auto type = glap::v2::ParameterType::Argument;
    using result_type = void;
};

constexpr auto to_int(std::string_view str) -> tl::expected<int, std::errc> {
    int result;
    auto [_, ec] { std::from_chars(str.data(), str.data() + str.size(), result) };
 
    if (ec == std::errc()) {
        return result;
    } else {
        return tl::make_unexpected(ec);
    }
}

int main(int argc, char** argv)
{
    glap::v2::Command<glap::v2::Names<"command", glap::v2::discard>, Param1> command;
    glap::v2::Command<glap::v2::Names<"command", 'c'>, Param1> command1;
    glap::v2::Command<glap::v2::Names<"command">, glap::v2::Argument<glap::v2::Names<"param", 'p'>, to_int>> command2;
    auto param = command2.get_parameter<"param">();
    
    auto value = param.resolve("std::string_view value");


    fmt::print("{}", decltype(command1)::names::longname);
}