#include "glap/common/error.h"
#include <array>
#include <charconv>
#include <glap/v2/parser.h>
#include <glap/v2/model.h>
#include <optional>
#include <string>
#include <string_view>
#include <concepts>
#include <gtest/gtest.h>
#include <variant>

namespace gl2 = glap::v2;

bool test_is_hello_world(std::string_view v) {
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

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

template <>
std::optional<Point> from_chars<std::optional<Point>>(std::string_view v) {
    Point result;
    auto pos = v.find(',');
    if (pos == std::string_view::npos) {
        return std::nullopt;
    }
    result.x = from_chars<int>(v.substr(0, pos));
    result.y = from_chars<int>(v.substr(pos + 1));
    return result;
}

using Command1 = gl2::model::Command<glap::v2::Names<"command1", 't'>, 
    gl2::model::Flag<glap::v2::Names<"flag", 'f'>>,
    gl2::model::Argument<glap::v2::Names<"arg", 'c'>>,
    gl2::model::Input<>
>;

using Command2 = gl2::model::Command<glap::v2::Names<"command2", gl2::discard>, 
    gl2::model::Flag<glap::v2::Names<"flag", 'f'>>,
    gl2::model::Argument<glap::v2::Names<"arg", 'a'>, gl2::discard, test_is_hello_world>,
    gl2::model::Arguments<glap::v2::Names<"args", 'b'>>,
    gl2::model::Inputs<>
>;

using Command3 = gl2::model::Command<glap::v2::Names<"command3", gl2::discard>, 
    gl2::model::Argument<glap::v2::Names<"float", gl2::discard>, from_chars<float>>,
    gl2::model::Argument<glap::v2::Names<"int", gl2::discard>, from_chars<int>>,
    gl2::model::Argument<glap::v2::Names<"point", gl2::discard>, from_chars<std::optional<Point>>>,
    gl2::model::Inputs<3>
>;

gl2::Parser<glap::v2::DefaultCommand::FirstDefined, Command1, Command2, Command3> parser;
gl2::Parser<glap::v2::DefaultCommand::None, Command1, Command2, Command3> parser_no_default;

using namespace std::literals;

TEST(glap_program, program_name) {
    auto result = parser.parse(std::array{"glap"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().program, "glap");
}

#pragma region Glap command tests
TEST(glap_command, command_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.longname(), "command1"sv);
}
TEST(glap_command, command_long_name2) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.longname(), "command2"sv);
}
TEST(glap_command, command_bad_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command_none"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "t"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.longname(), "command1"sv);
}
TEST(glap_command, command_bad_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "a"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_default) {
    auto result = parser.parse(std::array{"glap"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.longname(), "command1"sv);
}
TEST(glap_command, command_no_default) {
    auto result = parser_no_default.parse(std::array{"glap"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap flag tests
TEST(glap_flag, flag_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 1);
}
TEST(glap_flag, flag_long_name2) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "--flag_none"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownParameter);
}
TEST(glap_flag, flag_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "-f"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "-a"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownParameter);
}
TEST(glap_flag, multi_flag_longname) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "--flag"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "-f"sv, "-f"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname2) {
    auto result = parser.parse(std::array{"glap"sv, "command1"sv, "-fff"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 3);
}
TEST(glap_flag, flag_default_command) {
    auto result = parser.parse(std::array{"glap"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 1);
}
TEST(glap_flag, flag_no_default_command) {
    auto result = parser_no_default.parse(std::array{"glap"sv, "--flag"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap argument tests
TEST(glap_argument, argument_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--arg=hello"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"arg">().value, "hello");
}
TEST(glap_argument, argument_long_name2) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Argument 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_argument, argument_bad_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--arg_none=hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownParameter);
}
TEST(glap_argument, argument_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"arg">().value, "hello");
}
TEST(glap_argument, argument_bad_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-z"sv, "hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownParameter);
}
TEST(glap_argument, argument_already_set) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--arg=hello"sv, "--arg=world"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::AlreadySet);
}
TEST(glap_argument, multi_argument_long_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--args=value1"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Argument 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Argument 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_argument, multi_argument_short_name) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-b"sv, "value1"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Argument 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Argument 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_argument, multi_argument_short_name2) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-bb"sv, "value1"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Argument 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Argument 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_argument, argument_bad_validate) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--arg=not_hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::InvalidValue);
}
TEST(glap_argument, argument_default_command) {
    auto result = parser.parse(std::array{"glap"sv, "-c"sv, "value"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"arg">().value, "value");
}
TEST(glap_argument, argument_no_default_command) {
    auto result = parser_no_default.parse(std::array{"glap"sv,"--arg=value"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap argument resolve tests
TEST(glap_resolver, integer_argument) {
    auto result = parser.parse(std::array{"glap"sv, "command3"sv, "--int=123"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"int">().resolve(), std::optional{123});
}
TEST(glap_resolver, float_argument) {
    auto result = parser.parse(std::array{"glap"sv, "command3"sv, "--float=1.5"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"float">().resolve(), std::optional{1.5f});
}
TEST(glap_resolver, point_argument) {
    auto result = parser.parse(std::array{"glap"sv, "command3"sv, "--point=12,34"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    auto pt_test = Point{12, 34};
    ASSERT_EQ(command1.get_parameter<"point">().resolve(), std::optional{pt_test});
}
#pragma endregion

#pragma region Glap combined parameters tests
TEST(glap_combined, arguments) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "--arg=hello"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_parameter<"arg">().value, "hello");
    auto args = command.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Argument 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_combined, arguments_short) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_parameter<"arg">().value, "hello");
    auto args = command.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Argument 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_combined, arguments_short2) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-ab"sv, "hello"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_parameter<"arg">().value, "hello");
    auto args = command.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Argument 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Argument 'args' has wrong value";
}
TEST(glap_combined, argument_flags) {
    auto result = parser.parse(std::array{"glap"sv, "command2"sv, "-bbfbf"sv, "v1"sv, "v2"sv, "v3"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_parameter<"flag">().occurences, 2);
    auto args = command.get_parameter<"args">().values;
    ASSERT_FALSE(args.empty()) << "Argument 'args' is empty";
    ASSERT_EQ(args.size(), 3) << "Argument 'args' has not exactly 3 values";
    ASSERT_EQ(args[0].value, "v1"sv) << "Argument 'args' has wrong value";
    ASSERT_EQ(args[1].value, "v2"sv) << "Argument 'args' has wrong value";
    ASSERT_EQ(args[2].value, "v3"sv) << "Argument 'args' has wrong value";
}
TEST(glap_combined, arguments_default_command) {
    auto result = parser.parse(std::array{"glap"sv, "-c"sv, "value"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_parameter<"arg">().value, "value");
    ASSERT_EQ(command1.get_parameter<"flag">().occurences, 1);
}
#pragma endregion