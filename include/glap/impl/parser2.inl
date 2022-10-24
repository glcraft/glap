#pragma once
#include "../parser.h"
#include "../model.h"
#include <optional>
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
                            .type = Error::Type::None,
                            .code = Error::Code::NoGlobalCommand
                        },
                        .position = 0
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
                    if (res)
                        codepoint = res.value();
                    else 
                        return make_unexpected(PositionnedError{
                            .error = Error{
                                .parameter = "",
                                .value = std::nullopt,
                                .type = Error::Type::None,
                                .code = Error::Code::BadString
                            },
                            .position = 0
                        });
                } else {
                    codepoint = std::nullopt;
                }

                ([&]{
                    if (impl::check_names<Commands>(name, codepoint)) {
                        program.command.template emplace<Commands>();
                        result = glap::parse<Commands>.parse(std::get<Commands>(program.command), utils::BiIterator(itarg, params.end));
                        return true;
                    }
                    return false;
                }() || ...);
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
            
            return params.begin;
        }
    };
    template <class ArgNames>
    class Parser<model::Flag<ArgNames>> : public Parser<Parser<model::Flag<ArgNames>>> {
        using OutputType = model::Flag<ArgNames>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> params) const -> PosExpected<Iter>
        {

        }
    };
    template <class ArgNames, auto Resolver, auto Validator>
    class Parser<model::Parameter<ArgNames, Resolver, Validator>> : public Parser<Parser<model::Parameter<ArgNames, Resolver, Validator>>> {
        using OutputType = model::Parameter<ArgNames, Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
    template <class ArgNames, auto N, auto Resolver, auto Validator>
    class Parser<model::Parameters<ArgNames, N, Resolver, Validator>> : public Parser<Parser<model::Parameters<ArgNames, N, Resolver, Validator>>> {
        using OutputType = model::Parameters<ArgNames, N, Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
    template <auto Resolver, auto Validator>
    class Parser<model::Input<Resolver, Validator>> : public Parser<Parser<model::Input<Resolver, Validator>>> {
        using OutputType = model::Input<Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
    template <auto N, auto Resolver, auto Validator>
    class Parser<model::Inputs<N, Resolver, Validator>> : public Parser<Parser<model::Inputs<N, Resolver, Validator>>> {
        using OutputType = model::Inputs<N, Resolver, Validator>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
}