#pragma once
#include "../parser.h"
#include "../model.h"
#include "glap/core/expected.h"
#include <iterator>
#include <optional>
#include <string_view>
#include <variant>

namespace glap
{
    namespace impl {
        template <HasLongName Command>
        static constexpr bool check_names(std::optional<std::string_view> name, std::optional<char32_t> codepoint)
        {
            if (name && *name == Command::longname)
                return true;
            if constexpr(HasShortName<Command>)
                if (codepoint && *codepoint == *Command::shortname)
                    return true;
            return false;
        }
    }
    template <class Model>
    class Parser<Parser<Model>>
    {
        using BaseType = Parser<Model>;
        using OutputType = Model;
    public:
        template <class Iter>
        constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<OutputType>
        {
            OutputType result;
            auto cmd = static_cast<const BaseType*>(this)->parse(result, args);
            if (!cmd)
                return glap::make_unexpected(cmd.error());
            else
                return result;
        }
        constexpr auto operator()(utils::Range<std::string_view> auto args) const -> PosExpected<OutputType>
        {
            return operator()(utils::BiIterator{args.begin(), args.end()});
        }
    };
    struct ParsedParameter
    {
        std::optional<std::variant<std::string_view, char32_t>> name;
        std::optional<std::string_view> value;
    };
    template <StringLiteral Name, model::DefaultCommand def_cmd, class... Commands>
    class Parser<model::Program<Name, def_cmd, Commands...>> : public Parser<Parser<model::Program<Name, def_cmd, Commands...>>> {
    public:
        using OutputType = model::Program<Name, def_cmd, Commands...>;
        template <class Iter>
        constexpr auto parse(OutputType& program, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {
            if (args.size() == 0) [[unlikely]] {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .parameter = "",
                        .value = std::nullopt,
                        .type = Error::Type::None,
                        .code = Error::Code::NoParameter
                    },
                    .position = 0
                });
            }
            auto itarg = args.begin;
            program.program = *itarg++;
            auto default_command = [&] () {
                if (itarg == args.end) {
                    return true;
                }
                if (itarg->starts_with("-")) {
                    return true;
                }
                return false;
            }();
            PosExpected<Iter> result;
            if (default_command) {
                if constexpr (def_cmd == model::DefaultCommand::None) {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = "",
                            .value = std::nullopt,
                            .type = Error::Type::Command,
                            .code = Error::Code::NoGlobalCommand
                        },
                        .position = std::distance(args.begin, itarg)
                    });
                } else {
                    program.command.template emplace<0>();
                    result = glap::parser<std::variant_alternative_t<0, decltype(program.command)>>.parse(std::get<0>(program.command), utils::BiIterator(itarg, args.end));
                }
            }
            else {
                auto name = *itarg++;
                std::optional<char32_t> codepoint;
                if (utils::uni::utf8_length(name) == 1) {
                    auto res = utils::uni::codepoint(name);
                    if (res) [[likely]] 
                        codepoint = res.value();
                    else 
                        return make_unexpected(PositionnedError{
                            .error = Error{
                                .parameter = name,
                                .value = std::nullopt,
                                .type = Error::Type::Command,
                                .code = Error::Code::BadString
                            },
                            .position = std::distance(args.begin, itarg)
                        });
                } else {
                    codepoint = std::nullopt;
                }

                auto found = ([&]{
                    if (impl::check_names<Commands>(name, codepoint)) {
                        program.command.template emplace<Commands>();
                        result = glap::parser<Commands>.parse(std::get<Commands>(program.command), utils::BiIterator(itarg, args.end));
                        return true;
                    }
                    return false;
                }() || ...);
                if (!found) [[unlikely]] {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = name,
                            .value = std::nullopt,
                            .type = Error::Type::Command,
                            .code = Error::Code::BadCommand
                        },
                        .position = std::distance(args.begin, itarg)
                    });
                }
            }
            return result;
        }
    };
    template <HasLongName CommandNames, model::IsArgument... Arguments>
    class Parser<model::Command<CommandNames, Arguments...>> : public Parser<Parser<model::Command<CommandNames, Arguments...>>> {
    public:
        using OutputType = model::Command<CommandNames, Arguments...>;
        template <class Iter>
        constexpr auto parse(OutputType& command, utils::BiIterator<Iter> params) const -> PosExpected<Iter>
        {
            auto itcurrent = params.begin;
            while(itcurrent != params.end) {
                auto arg = *itcurrent;
                if (arg.starts_with("---")) {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = arg,
                            .value = std::nullopt,
                            .type = Error::Type::None,
                            .code = Error::Code::SyntaxError
                        },
                        .position = std::distance(params.begin, itcurrent)
                    });
                } else {
                    PosExpected<Iter> res;
                    if (arg.starts_with("--")) {
                        res = parse_long<Iter>(command, {itcurrent, params.end});
                    } else if (arg.starts_with("-")) {
                        res = parse_short<Iter>(command, {itcurrent, params.end});
                    } else {
                        auto res_input = parse_input(command, arg);
                        if (!res_input) [[unlikely]] {
                            res = make_unexpected(PositionnedError{
                                .error = res_input.error(),
                                .position = std::distance(params.begin, itcurrent)
                            });
                        } else {
                            res = std::next(itcurrent);
                        }
                    }
                    if (!res) [[unlikely]] {
                        return res;
                    } 
                    itcurrent = res.value();
                }
            }
            return params.begin;
        }
    private:
        template <class Iter>
        constexpr auto parse_long(OutputType& command, utils::BiIterator<Iter> params) const -> PosExpected<Iter>
        {
            auto arg = *params.begin++;
            auto name_value = arg.substr(2);
            auto pos_equal = name_value.find('=');
            auto name = name_value.substr(0, pos_equal);
            Expected<void> res;
            bool found = false;
            if (pos_equal == std::string_view::npos) {
                found = ([&]{
                    if constexpr(glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Flag>) {
                        if (impl::check_names<Arguments>(name, std::nullopt)) {
                            res = glap::parser<Arguments>.parse(std::get<Arguments>(command.arguments));
                            return true;
                        }
                    }
                    return false;
                }() || ...);
            } else {
                auto value = name_value.substr(pos_equal + 1);
                found = ([&]{
                    if constexpr(glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Parameter>) {
                        if (impl::check_names<Arguments>(name, std::nullopt)) {
                            res = glap::parser<Arguments>.parse(std::get<Arguments>(command.arguments), value);
                            return true;
                        }
                    }
                    return false;
                }() || ...);
            }
            if (!found) [[unlikely]] {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .parameter = name,
                        .value = std::nullopt,
                        .type = Error::Type::Unknown,
                        .code = Error::Code::UnknownArgument
                    },
                    .position = 0
                });
            }
            if (!res) {
                return make_unexpected(PositionnedError{
                    .error = res.error(),
                    .position = 0
                });
            }
            return params.begin;
        }
        template <class Iter>
        constexpr auto parse_short(OutputType& command, utils::BiIterator<Iter> params) const -> PosExpected<Iter>
        {
            auto itcurrent = params.begin;
            auto arg = *itcurrent++;
            auto list_names = arg.substr(1);
            auto len_res = utils::uni::utf8_char_length(std::string_view(list_names));
            if (!len_res) [[unlikely]] {
                return make_unexpected(PositionnedError{
                    .error = Error{
                        .parameter = list_names,
                        .value = std::nullopt,
                        .type = Error::Type::Unknown,
                        .code = Error::Code::BadString
                    },
                    .position = std::distance(params.begin, itcurrent)
                });
            }
            auto len = len_res.value();
            auto ch = list_names;
            for(auto it = list_names.begin(); it != list_names.end(); it = std::next(it,len ), ch=std::string_view(it, list_names.end())) {
                len_res = utils::uni::utf8_char_length(ch);
                if (!len_res) [[unlikely]] {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = ch,
                            .value = std::nullopt,
                            .type = Error::Type::Unknown,
                            .code = Error::Code::BadString
                        },
                        .position = std::distance(params.begin, itcurrent)
                    });
                }
                len = len_res.value();
                auto codepoint_res = utils::uni::codepoint(ch);
                if (!codepoint_res) [[unlikely]] {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = ch,
                            .value = std::nullopt,
                            .type = Error::Type::Unknown,
                            .code = Error::Code::BadString
                        },
                        .position = std::distance(params.begin, itcurrent)
                    });
                }
                auto codepoint = codepoint_res.value();
                Expected<void> res;

                bool found = ([&]{
                    if constexpr(!glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Input>) {
                        if (impl::check_names<Arguments>(std::nullopt, codepoint)) {
                            if constexpr(glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Flag>) {
                                res = glap::parser<Arguments>.parse(std::get<Arguments>(command.arguments));
                            } else if constexpr(glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Parameter>) {
                                if (itcurrent == params.end) {
                                    res = make_unexpected(Error{
                                        .parameter = ch,
                                        .value = std::nullopt,
                                        .type = Error::Type::Parameter,
                                        .code = Error::Code::MissingValue
                                    });
                                } else {
                                    res = glap::parser<Arguments>.parse(std::get<Arguments>(command.arguments), *itcurrent++);
                                }
                            }
                            return true;
                        }
                    }
                    return false;
                }() || ...);

                if (!found) [[unlikely]] {
                    return make_unexpected(PositionnedError{
                        .error = Error{
                            .parameter = arg,
                            .value = std::nullopt,
                            .type = Error::Type::Unknown,
                            .code = Error::Code::UnknownArgument
                        },
                        .position = 0
                    });
                }
                if (!res) {
                    return make_unexpected(PositionnedError{
                        .error = res.error(),
                        .position = std::distance(params.begin, itcurrent)
                    });
                }
            }

            return itcurrent;
        }

        constexpr auto parse_input(OutputType& command, std::string_view input) const -> Expected<void>
        {
            Expected<void> res;
            auto found = ([&] {
                if constexpr(glap::model::IsArgumentTyped<Arguments, glap::model::ArgumentType::Input>) {
                    res = glap::parser<Arguments>.parse(std::get<Arguments>(command.arguments), input);
                    return true;
                } else {
                    return false;
                }
            }() || ...);
            if (!found) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = "",
                    .value = std::nullopt,
                    .type = Error::Type::Input,
                    .code = Error::Code::UnknownArgument
                });
            }
            return res;
        }
    };
    template <class ArgNames>
    class Parser<model::Flag<ArgNames>> : public Parser<Parser<model::Flag<ArgNames>>> {
        using OutputType = model::Flag<ArgNames>;
    public:
        constexpr auto parse(OutputType& flag) const -> Expected<void>
        {
            flag.occurences++;
            return {};
        }
    };
    template <class OutputType, auto Resolver, auto Validator>
    auto check_value(std::string_view value) -> Expected<OutputType>
    {
        if constexpr (IsValidator<decltype(Validator)>) {
            if (!Validator(value)) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::BadValidation
                });
            }
        }
        if constexpr (IsResolver<decltype(Resolver)>) {
            auto result = Resolver(value);
            if constexpr (IsExpected<decltype(result)>) {
                if (!result) [[unlikely]] {
                    return make_unexpected(Error{
                        .parameter = std::string_view(),
                        .value = value,
                        .type = Error::Type::Parameter,
                        .code = Error::Code::BadResolution
                    });
                }
            }
            return *result;
        } else {
            return value;
        }
    }
    template <class ArgNames, auto Resolver, auto Validator>
    class Parser<model::Parameter<ArgNames, Resolver, Validator>> {
        using OutputType = model::Parameter<ArgNames, Resolver, Validator>;
    public:
        constexpr auto parse(OutputType& arg, std::string_view value) const -> Expected<void>
        {
            if (arg.value.has_value()) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::DuplicateParameter
                });
            }
            auto result = check_value<typename OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            arg.value = std::move(result.value());
            return {};
        }
    };
    template <class ArgNames, auto N, auto Resolver, auto Validator>
    class Parser<model::Parameters<ArgNames, N, Resolver, Validator>> {
        using OutputType = model::Parameters<ArgNames, N, Resolver, Validator>;
    public:
        constexpr auto parse(OutputType& params, std::string_view value) const -> Expected<void>
        {
            if constexpr (!std::same_as<std::remove_cv_t<decltype(N)>, Discard>) {
                if (params.values.size() >= N) [[unlikely]] {
                    return make_unexpected(Error{
                        .parameter = std::string_view(),
                        .value = value,
                        .type = Error::Type::Parameter,
                        .code = Error::Code::TooManyParameters
                    });
                }
            }
            auto result = check_value<typename OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            params.values.push_back(std::move(result.value()));
            return {};
        }
    };
    template <auto Resolver, auto Validator>
    class Parser<model::Input<Resolver, Validator>> {
        using OutputType = model::Input<Resolver, Validator>;
    public:
        constexpr auto parse(OutputType& input, std::string_view value) const -> Expected<void>
        {
            if (input.value.has_value()) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::DuplicateParameter
                });
            }
            auto result = check_value<typename OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            input.value = std::move(result.value());
            return {};
        }
    };
    template <auto N, auto Resolver, auto Validator>
    class Parser<model::Inputs<N, Resolver, Validator>> {
        using OutputType = model::Inputs<N, Resolver, Validator>;
    public:
        constexpr auto parse(OutputType& inputs, std::string_view value) const -> Expected<void>
        {
            if constexpr (!std::same_as<std::remove_cv_t<decltype(N)>, Discard>) {
                if (inputs.values.size() >= N) [[unlikely]] {
                    return make_unexpected(Error{
                        .parameter = std::string_view(),
                        .value = value,
                        .type = Error::Type::Parameter,
                        .code = Error::Code::TooManyParameters
                    });
                }
            }
            auto result = check_value<typename OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            inputs.values.push_back(std::move(result.value()));
            return {};
        }
    };
}