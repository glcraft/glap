#include "tl/expected.hpp"
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
tl::expected<T, glap::Discard> from_chars(std::string_view v) {
    T result;
    auto ch_result = std::from_chars(v.data(), v.data() + v.size(), result);
    if (ch_result.ec == std::errc()) {
        return result;
    } else {
        return tl::make_unexpected(glap::discard);
    }
}

template <>
tl::expected<float, glap::Discard> from_chars<float>(std::string_view v) {
    try {
        return std::stof(std::string(v));
    } catch (const std::invalid_argument&) {
        return tl::make_unexpected(glap::discard);
    }
}
template <>
tl::expected<double, glap::Discard> from_chars<double>(std::string_view v) {
    try {
        return std::stod(std::string(v));
    } catch (const std::invalid_argument&) {
        return tl::make_unexpected(glap::discard);
    }
}
template <>
tl::expected<long double, glap::Discard> from_chars<long double>(std::string_view v) {
    try {
        return std::stold(std::string(v));
    } catch (const std::invalid_argument&) {
        return tl::make_unexpected(glap::discard);
    }
}

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

template <>
tl::expected<Point, glap::Discard> from_chars<Point>(std::string_view v) {
    Point result;
    auto pos = v.find(',');
    if (pos == std::string_view::npos) {
        return tl::make_unexpected(glap::discard);
    }
    auto ch_result = std::from_chars(v.data(), v.data() + pos, result.x);
    if (ch_result.ec != std::errc()) {
        return tl::make_unexpected(glap::discard);
    }
    ch_result = std::from_chars(v.data() + pos + 1, v.data() + v.size(), result.y);
    if (ch_result.ec != std::errc()) {
        return tl::make_unexpected(glap::discard);
    }
    return result;
}

using Command1 = glap::model::Command<glap::Names<"command1", 't'>, 
    glap::model::Flag<glap::Names<"flag", 'f'>>,
    glap::model::Parameter<glap::Names<"param", 'c'>>,
    glap::model::Input<>
>;

using Command2 = glap::model::Command<glap::Names<"command2", glap::discard>, 
    glap::model::Flag<glap::Names<"flag", 'f'>>,
    glap::model::Parameter<glap::Names<"param", 'a'>, glap::discard, test_is_hello_world>,
    glap::model::Parameters<glap::Names<"params", 'b'>>,
    glap::model::Parameters<glap::Names<"fixed_args", 'c'>, 2>,
    glap::model::Inputs<>
>;

using Command3 = glap::model::Command<glap::Names<"command3", glap::discard>, 
    glap::model::Parameter<glap::Names<"float", glap::discard>, from_chars<float>>,
    glap::model::Parameter<glap::Names<"int", glap::discard>, from_chars<int>>,
    glap::model::Parameter<glap::Names<"point", glap::discard>, from_chars<Point>>,
    glap::model::Inputs<2>
>;

using Command4 = glap::model::Command<glap::Names<"command4">, 
    glap::model::Input<glap::discard, test_is_hello_world>
>;
using Command5 = glap::model::Command<glap::Names<"command5">, 
    glap::model::Input<from_chars<int>>
>;

using ProgramTest = glap::model::Program<"test", glap::model::DefaultCommand::FirstDefined, Command1, Command2, Command3, Command4, Command5>;
using ProgramTestNoCommand = glap::model::Command<glap::Names<"test_no_command">, glap::model::Inputs<>>;
using ProgramTestNoDefault = glap::model::Program<"test_no_default", glap::model::DefaultCommand::None, Command1, Command2, Command3>;

constexpr auto tests_parser = glap::parser<ProgramTest>;
constexpr auto tests_parser_no_default = glap::parser<ProgramTestNoDefault>;
constexpr auto tests_no_command = glap::parser<ProgramTestNoCommand>;


using namespace std::literals;

TEST(glap_program, program_name) {
    auto result = tests_parser(std::array{"glap"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().program, "glap");
}

#pragma region Glap command tests
TEST(glap_command, command_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_long_name2) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
}
TEST(glap_command, command_bad_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command_none"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "t"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_bad_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "a"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadCommand);
}
TEST(glap_command, command_default) {
    auto result = tests_parser(std::array{"glap"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
}
TEST(glap_command, command_no_default) {
    auto result = tests_parser_no_default(std::array{"glap"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
TEST(glap_command, program_no_command) {
    auto args = std::array{"glap"sv, "input1"sv, "input2"sv};
    auto result = tests_no_command(std::span(args).subspan(1));
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().get_inputs().size(), 2) << "Wrong number of inputs";
    ASSERT_TRUE(result.value().get_inputs()[0].value) << "Input 1 is not set";
    ASSERT_TRUE(result.value().get_inputs()[1].value) << "Input 2 is not set";
    ASSERT_EQ(result.value().get_inputs()[0].value.value(), "input1") << "Wrong input";
    ASSERT_EQ(result.value().get_inputs()[1].value.value(), "input2") << "Wrong input";
}
#pragma endregion

#pragma region Glap flag tests
TEST(glap_flag, flag_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "--flag"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_long_name2) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--flag"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "--flag_none"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_flag, flag_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "-f"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_bad_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "-a"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_flag, multi_flag_longname) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "--flag"sv, "--flag"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "-f"sv, "-f"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 2);
}
TEST(glap_flag, multi_flag_shortname2) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "-fff"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 3);
}
TEST(glap_flag, flag_default_command) {
    auto result = tests_parser(std::array{"glap"sv, "--flag"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
TEST(glap_flag, flag_no_default_command) {
    auto result = tests_parser_no_default(std::array{"glap"sv, "--flag"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap parameter tests
TEST(glap_parameter, parameter_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--param=hello"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"param">().value, "hello");
}
TEST(glap_parameter, parameter_long_name2) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--params=value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto params = command1.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 1) << "Parameter 'params' has more than one value";
    ASSERT_EQ(params[0].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_parameter, parameter_bad_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--arg_none=hello"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_parameter, parameter_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command1.get_argument<"param">().value, "hello");
}
TEST(glap_parameter, parameter_bad_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-z"sv, "hello"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::UnknownArgument);
}
TEST(glap_parameter, parameter_already_set) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--param=hello"sv, "--param=world"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::DuplicateParameter);
}
TEST(glap_parameter, multi_parameter_long_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--params=value1"sv, "--params=value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto params = command1.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 2) << "Parameter 'params' has more than two values";
    ASSERT_EQ(params[0].value, "value1"sv) << "Parameter 'params' has wrong value";
    ASSERT_EQ(params[1].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_parameter, multi_parameter_short_name) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-b"sv, "value1"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto params = command1.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 2) << "Parameter 'params' has more than two values";
    ASSERT_EQ(params[0].value, "value1"sv) << "Parameter 'params' has wrong value";
    ASSERT_EQ(params[1].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_parameter, multi_parameter_short_name2) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-bb"sv, "value1"sv, "value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command1 = std::get<Command2>(result.value().command);
    auto params = command1.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 2) << "Parameter 'params' has more than two values";
    ASSERT_EQ(params[0].value, "value1"sv) << "Parameter 'params' has wrong value";
    ASSERT_EQ(params[1].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_parameter, multi_parameter_out_of_range) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-ccc"sv, "v1"sv, "v2"sv, "v3"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::TooManyParameters);
}
TEST(glap_parameter, parameter_bad_validate) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--param=not_hello"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadValidation);
}
TEST(glap_parameter, parameter_default_command) {
    auto result = tests_parser(std::array{"glap"sv, "-c"sv, "value"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"param">().value, "value");
}
TEST(glap_parameter, parameter_no_default_command) {
    auto result = tests_parser_no_default(std::array{"glap"sv,"--param=value"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::NoGlobalCommand);
}
#pragma endregion

#pragma region Glap inputs tests
TEST(glap_inputs, single_input) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "input1"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_inputs().value.value(), "input1");
}
TEST(glap_inputs, single_input_already_set) {
    auto result = tests_parser(std::array{"glap"sv, "command1"sv, "input1"sv, "input2"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::DuplicateParameter);
}
TEST(glap_inputs, multi_inputs) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "input1"sv, "input2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command2 = std::get<Command2>(result.value().command);
    ASSERT_EQ(command2.get_inputs().values[0].value.value(), "input1");
    ASSERT_EQ(command2.get_inputs().values[1].value.value(), "input2");
}
TEST(glap_inputs, fixed_multi_inputs) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "input1"sv, "input2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command3 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command3.get_inputs().values[0].value.value(), "input1");
    ASSERT_EQ(command3.get_inputs().values[1].value.value(), "input2");
}
TEST(glap_inputs, inputs_out_of_range) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "input1"sv, "input2"sv, "input3"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::TooManyParameters);
}
TEST(glap_inputs, inputs_validation) {
    auto result = tests_parser(std::array{"glap"sv, "command4"sv, "hello"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 3) << "Wrong command index";
    auto command4 = std::get<Command4>(result.value().command);
    ASSERT_EQ(command4.get_inputs().value, "hello");
}
TEST(glap_inputs, inputs_bad_validation) {
    auto result = tests_parser(std::array{"glap"sv, "command4"sv, "not_hello"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadValidation);
}
TEST(glap_inputs, inputs_resolution) {
    auto result = tests_parser(std::array{"glap"sv, "command5"sv, "123"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 4) << "Wrong command index";
    auto command5 = std::get<Command5>(result.value().command);
    ASSERT_EQ(command5.get_inputs().value, 123);
}
TEST(glap_inputs, inputs_bad_resolution) {
    auto result = tests_parser(std::array{"glap"sv, "command5"sv, "not_a_number"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadResolution);
}


#pragma endregion

#pragma region Glap parameter resolve tests
TEST(glap_resolver, integer_parameter) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "--int=123"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_argument<"int">().value, std::optional{123});
}
TEST(glap_resolver, float_parameter) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "--float=1.5"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    ASSERT_EQ(command1.get_argument<"float">().value, std::optional{1.5f});
}
TEST(glap_resolver, point_parameter) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "--point=12,34"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 2) << "Wrong command index";
    auto command1 = std::get<Command3>(result.value().command);
    auto pt_test = Point{12, 34};
    ASSERT_EQ(command1.get_argument<"point">().value, std::optional{pt_test});
}
TEST(glap_resolver, integer_parameter_bad_resolution) {
    auto result = tests_parser(std::array{"glap"sv, "command3"sv, "--int=not_a_number"sv});
    ASSERT_FALSE(result) << "Parser successed when it should not";
    auto error = result.error();
    ASSERT_EQ(error.error.code, glap::Error::Code::BadResolution);
}
#pragma endregion

#pragma region Glap combined arguments tests
TEST(glap_combined, parameters) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "--param=hello"sv, "--params=value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"param">().value, "hello");
    auto params = command.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 1) << "Parameter 'params' has more than one value";
    ASSERT_EQ(params[0].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_combined, parameters_short) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-a"sv, "hello"sv, "-b"sv, "value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"param">().value, "hello");
    auto params = command.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 1) << "Parameter 'params' has more than one value";
    ASSERT_EQ(params[0].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_combined, parameters_short2) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-ab"sv, "hello"sv, "value2"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"param">().value, "hello");
    auto params = command.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 1) << "Parameter 'params' has more than one value";
    ASSERT_EQ(params[0].value, "value2"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_combined, parameter_flags) {
    auto result = tests_parser(std::array{"glap"sv, "command2"sv, "-bbfbf"sv, "v1"sv, "v2"sv, "v3"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 1) << "Wrong command index";
    auto command = std::get<Command2>(result.value().command);
    EXPECT_EQ(command.get_argument<"flag">().occurences, 2);
    auto params = command.get_argument<"params">().values;
    ASSERT_FALSE(params.empty()) << "Parameter 'params' is empty";
    ASSERT_EQ(params.size(), 3) << "Parameter 'params' has not exactly 3 values";
    ASSERT_EQ(params[0].value, "v1"sv) << "Parameter 'params' has wrong value";
    ASSERT_EQ(params[1].value, "v2"sv) << "Parameter 'params' has wrong value";
    ASSERT_EQ(params[2].value, "v3"sv) << "Parameter 'params' has wrong value";
}
TEST(glap_combined, parameters_default_command) {
    auto result = tests_parser(std::array{"glap"sv, "-c"sv, "value"sv, "--flag"sv});
    ASSERT_TRUE(result) << "Parser failed: " << result.error().to_string();
    ASSERT_EQ(result.value().command.index(), 0) << "Wrong command index";
    auto command1 = std::get<Command1>(result.value().command);
    ASSERT_EQ(command1.get_argument<"param">().value, "value");
    ASSERT_EQ(command1.get_argument<"flag">().occurences, 1);
}
#pragma endregion