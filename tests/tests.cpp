#include <array>
#include <charconv>
#include <glap/parser.h>
#include <glap/model.h>
#include <optional>
#include <string>
#include <string_view>
#include <concepts>
#include <gtest/gtest.h>
#include <variant>

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

using Command1 = glap::model::Command<glap::Names<"command1", 't'>, 
    glap::model::Flag<glap::Names<"flag", 'f'>>,
    glap::model::Parameter<glap::Names<"arg", 'c'>>,
    glap::model::Input<>
>;

using Command2 = glap::model::Command<glap::Names<"command2", glap::discard>, 
    glap::model::Flag<glap::Names<"flag", 'f'>>,
    glap::model::Parameter<glap::Names<"arg", 'a'>, glap::discard, test_is_hello_world>,
    glap::model::Parameters<glap::Names<"args", 'b'>>,
    glap::model::Inputs<>
>;

using Command3 = glap::model::Command<glap::Names<"command3", glap::discard>, 
    glap::model::Parameter<glap::Names<"float", glap::discard>, from_chars<float>>,
    glap::model::Parameter<glap::Names<"int", glap::discard>, from_chars<int>>,
    glap::model::Parameter<glap::Names<"point", glap::discard>, from_chars<std::optional<Point>>>,
    glap::model::Inputs<3>
>;

using ProgramTest = glap::model::Program<"test", glap::model::DefaultCommand::FirstDefined, Command1, Command2, Command3>;
using ProgramTestNoDefault = glap::model::Program<"test_no_default", glap::model::DefaultCommand::None, Command1, Command2, Command3>;

constexpr auto parser = glap::parse<ProgramTest>;
constexpr auto parser_no_default = glap::parse<ProgramTestNoDefault>;


using namespace std::literals;

TEST(glap_program, program_name) {
    auto result = parser(std::array{"glap"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().program, "glap");
}

#pragma region Glap command tests
TEST(glap_command, command_long_name) {
    auto result = parser(std::array{"glap"sv, "command1"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_long_name2) {
    auto result = parser(std::array{"glap"sv, "command2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
}
TEST(glap_command, command_bad_long_name) {
    auto result = parser(std::array{"glap"sv, "command_none"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_short_name) {
    auto result = parser(std::array{"glap"sv, "t"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_bad_short_name) {
    auto result = parser(std::array{"glap"sv, "a"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_default) {
    auto result = parser(std::array{"glap"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_no_default) {
    auto result = parser_no_default(std::array{"glap"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap flag tests
TEST(glap_flag, flag_long_name) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_long_name2) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_long_name) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "--flag_none"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_flag, flag_short_name) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "-f"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_short_name) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "-a"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_flag, multi_flag_longname) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "--flag"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "-f"sv, "-f"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname2) {
    auto result = parser(std::array{"glap"sv, "command1"sv, "-fff"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 3);
}
TEST(glap_flag, flag_default_command) {
    auto result = parser(std::array{"glap"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_no_default_command) {
    auto result = parser_no_default(std::array{"glap"sv, "--flag"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap parameter tests
TEST(glap_parameter, parameter_long_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--arg=hello"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"arg">().value, "hello");
}
TEST(glap_parameter, parameter_long_name2) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Parameter 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_parameter, parameter_bad_long_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--arg_none=hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_parameter, parameter_short_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"arg">().value, "hello");
}
TEST(glap_parameter, parameter_bad_short_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-z"sv, "hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_parameter, parameter_already_set) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--arg=hello"sv, "--arg=world"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::AlreadySet);
}
TEST(glap_parameter, multi_parameter_long_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--args=value1"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Parameter 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Parameter 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_parameter, multi_parameter_short_name) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-b"sv, "value1"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Parameter 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Parameter 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_parameter, multi_parameter_short_name2) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-bb"sv, "value1"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto args = command1.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 2) << "Parameter 'args' has more than two values";
    ASSERT_EQ(args[0].value, "value1"sv) << "Parameter 'args' has wrong value";
    ASSERT_EQ(args[1].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_parameter, parameter_bad_validate) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--arg=not_hello"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::InvalidValue);
}
TEST(glap_parameter, parameter_default_command) {
    auto result = parser(std::array{"glap"sv, "-c"sv, "value"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"arg">().value, "value");
}
TEST(glap_parameter, parameter_no_default_command) {
    auto result = parser_no_default(std::array{"glap"sv,"--arg=value"sv});
    ASSERT_TRUE(!result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap parameter resolve tests
TEST(glap_resolver, integer_parameter) {
    auto result = parser(std::array{"glap"sv, "command3"sv, "--int=123"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_argument<"int">().value, std::optional{123});
}
TEST(glap_resolver, float_parameter) {
    auto result = parser(std::array{"glap"sv, "command3"sv, "--float=1.5"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_argument<"float">().value, std::optional{1.5f});
}
TEST(glap_resolver, point_parameter) {
    auto result = parser(std::array{"glap"sv, "command3"sv, "--point=12,34"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    auto pt_test = Point{12, 34};
    ASSERT_EQ(command1.get_argument<"point">().value, std::optional{pt_test});
}
#pragma endregion

#pragma region Glap combined arguments tests
TEST(glap_combined, parameters) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "--arg=hello"sv, "--args=value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"arg">().value, "hello");
    auto args = command.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Parameter 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_combined, parameters_short) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"arg">().value, "hello");
    auto args = command.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Parameter 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_combined, parameters_short2) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-ab"sv, "hello"sv, "value2"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"arg">().value, "hello");
    auto args = command.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 1) << "Parameter 'args' has more than one value";
    ASSERT_EQ(args[0].value, "value2"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_combined, parameter_flags) {
    auto result = parser(std::array{"glap"sv, "command2"sv, "-bbfbf"sv, "v1"sv, "v2"sv, "v3"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"flag">().occurences, 2);
    auto args = command.get_argument<"args">().values;
    ASSERT_FALSE(args.empty()) << "Parameter 'args' is empty";
    ASSERT_EQ(args.size(), 3) << "Parameter 'args' has not exactly 3 values";
    ASSERT_EQ(args[0].value, "v1"sv) << "Parameter 'args' has wrong value";
    ASSERT_EQ(args[1].value, "v2"sv) << "Parameter 'args' has wrong value";
    ASSERT_EQ(args[2].value, "v3"sv) << "Parameter 'args' has wrong value";
}
TEST(glap_combined, parameters_default_command) {
    auto result = parser(std::array{"glap"sv, "-c"sv, "value"sv, "--flag"sv});
    ASSERT_TRUE(result.has_value()) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"arg">().value, "value");
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
#pragma endregion