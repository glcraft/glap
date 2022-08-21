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
    using namespace glap::v2;
    glap::v2::Command<glap::v2::Names<"command", glap::v2::discard>, Param1> command;
    glap::v2::Command<glap::v2::Names<"command", 'c'>, 
        glap::v2::Flag<glap::v2::Names<"flag", 'f'>>,
        glap::v2::Arguments<glap::v2::Names<"flags", 'a'>, 4>,
        glap::v2::Argument<glap::v2::Names<"integer", 'g'>, to_int>,
        glap::v2::Inputs<>
    > command1;

    // auto test = command1.get_parameter<"argument">().get<3>();

    fmt::print("{}", command1.longname());
    if (auto val = command1.get_parameter<"integer">().resolve(); val) {
        fmt::print("{}", *val);
    }
}