#include <__concepts/arithmetic.h>
#include <charconv>
#include <glap/v2/parser.h>
#include <glap/v2/model.h>
#include <string>
#include <string_view>
#include <concepts>

namespace gl2 = glap::v2;

bool is_hello_world(std::string_view v) {
    return v == "hello" || v == "world";
}

template <class T>
T from_chars(std::string_view v) {
    T result;
    std::from_chars(v.data(), v.data() + v.size(), result);
    return result;
}

template <>
float from_chars<float>(std::string_view v) {
    return std::stof(std::string(v));
}
template <>
double from_chars<double>(std::string_view v) {
    return std::stod(std::string(v));
}
template <>
long double from_chars<long double>(std::string_view v) {
    return std::stold(std::string(v));
}

gl2::Parser<glap::v2::DefaultCommand::FirstDefined, 
    gl2::model::Command<glap::v2::Names<"command1", 't'>, 
        gl2::model::Flag<glap::v2::Names<"flag", 'f'>>,
        gl2::model::Input<>
    >,
    gl2::model::Command<glap::v2::Names<"command2", gl2::discard>, 
        gl2::model::Flag<glap::v2::Names<"flag", 'f'>>,
        gl2::model::Argument<glap::v2::Names<"arg", 'a'>, gl2::discard, is_hello_world>,
        gl2::model::Arguments<glap::v2::Names<"args", 'b'>>,
        gl2::model::Inputs<>
    >,
    gl2::model::Command<glap::v2::Names<"command3", gl2::discard>, 
        gl2::model::Argument<glap::v2::Names<"float", gl2::discard>, from_chars<float>>,
        gl2::model::Argument<glap::v2::Names<"int", gl2::discard>, from_chars<int>>,
        gl2::model::Argument<glap::v2::Names<"sizet", gl2::discard>, from_chars<size_t>>,
        gl2::model::Inputs<3>
    >

> parser;

