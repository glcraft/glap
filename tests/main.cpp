#include "glap/v2/parser.h"
#include "tl/expected.hpp"
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
    Parser<
        glap::v2::Command<glap::v2::Names<"commands", glap::v2::discard>, Param1>,
        glap::v2::Command<glap::v2::Names<"command", 'c'>, 
            glap::v2::Flag<glap::v2::Names<"flag", 'f'>>,
            glap::v2::Arguments<glap::v2::Names<"flags", 'a'>, 4>,
            glap::v2::Argument<glap::v2::Names<"integer", 'g'>, to_int>,
            glap::v2::Inputs<>
    >> parser;
    
    auto result = parser.parse(std::span{argv, argv+argc});

    if (result) {
        fmt::print("{}\n", result.value().program);
    } else {
        fmt::print("{}\n", result.error().to_string());
    }
}