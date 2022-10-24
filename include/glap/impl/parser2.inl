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
        static constexpr bool check_names(std::string_view name, std::optional<char32_t> codepoint)
        {
            if (name == Command::longname)
                return true;
            if constexpr(HasShortName<Command>)
                if (codepoint && codepoint.value() == Command::shortname.value())
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
            constexpr auto parser = BaseType{};
            auto cmd = static_cast<const BaseType*>(this)->parse(result, args);
            if (!cmd)
                return glap::make_unexpected(cmd.error());
            else
                return result;
        }
        constexpr auto operator()(utils::Iterable<std::string_view> auto args) const -> PosExpected<OutputType>
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
        constexpr auto parse(OutputType& program, utils::BiIterator<Iter> params) const -> PosExpected<Iter>
        {
            if (params.size() == 0) [[unlikely]] {
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
            auto itarg = params.begin;
            program.program = *itarg++;
            auto default_command = [&] () {
                if (itarg == params.end) {
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
                        .position = std::distance(params.begin, itarg)
                    });
                }
                program.command.template emplace<0>();
                result = glap::parse<std::variant_alternative_t<0, decltype(program.command)>>.parse(std::get<0>(program.command), utils::BiIterator(itarg, params.end));
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
                            .position = std::distance(params.begin, itarg)
                        });
                } else {
                    codepoint = std::nullopt;
                }

                auto found = ([&]{
                    if (impl::check_names<Commands>(name, codepoint)) {
                        program.command.template emplace<Commands>();
                        result = glap::parse<Commands>.parse(std::get<Commands>(program.command), utils::BiIterator(itarg, params.end));
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
                        .position = std::distance(params.begin, itarg)
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
            while(params.begin != params.end) {
                
            }
            return params.begin;
        }
    };
    template <class ArgNames>
    class Parser<model::Flag<ArgNames>> : public Parser<Parser<model::Flag<ArgNames>>> {
        using OutputType = model::Flag<ArgNames>;
    public:
        template <class Iter>
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
        if constexpr (IsResolver<decltype(Resolver), OutputType>) {
            auto result = Resolver(value);
            if (!result) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::BadResolution
                });
            }
            return std::move(result);
        }
        return std::move(value);
    }
    template <class ArgNames, class T, auto Resolver, auto Validator>
    class Parser<model::Parameter<ArgNames, T, Resolver, Validator>> : public Parser<Parser<model::Parameter<ArgNames, T, Resolver, Validator>>> {
        using OutputType = model::Parameter<ArgNames, T, Resolver, Validator>;
    public:
        template <class Iter>
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
            auto result = check_value<OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            arg.value = std::move(result.value());
            return {};
        }
    };
    template <class ArgNames, class T, auto N, auto Resolver, auto Validator>
    class Parser<model::Parameters<ArgNames, T, N, Resolver, Validator>> : public Parser<Parser<model::Parameters<ArgNames, T, N, Resolver, Validator>>> {
        using OutputType = model::Parameters<ArgNames, T, N, Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType& params, std::string_view value) const -> Expected<void>
        {
            if (params.values.size() >= N) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::TooManyParameters
                });
            }
            auto result = check_value<OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            params.values.push_back(std::move(result.value()));
            return {};
        }
    };
    template <class T, auto Resolver, auto Validator>
    class Parser<model::Input<T, Resolver, Validator>> : public Parser<Parser<model::Input<T, Resolver, Validator>>> {
        using OutputType = model::Input<T, Resolver, Validator>;
    public:
        template <class Iter>
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
            auto result = check_value<OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            input.value = std::move(result.value());
            return {};
        }
    };
    template <class T, auto N, auto Resolver, auto Validator>
    class Parser<model::Inputs<T, N, Resolver, Validator>> : public Parser<Parser<model::Inputs<T, N, Resolver, Validator>>> {
        using OutputType = model::Inputs<T, N, Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType& inputs, std::string_view value) const -> Expected<void>
        {
            if (inputs.values.size() >= N) [[unlikely]] {
                return make_unexpected(Error{
                    .parameter = std::string_view(),
                    .value = value,
                    .type = Error::Type::Parameter,
                    .code = Error::Code::TooManyParameters
                });
            }
            auto result = check_value<OutputType::value_type, Resolver, Validator>(value);
            if (!result) [[unlikely]] {
                return make_unexpected(result.error());
            }
            inputs.values.push_back(std::move(result.value()));
            return {};
        }
    };
}