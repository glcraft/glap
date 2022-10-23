#pragma once
#include "../parser.h"
#include "../model.h"

namespace glap
{
    template <class Model>
    class Parser<Parser<Model>>
    {
        using BaseType = Parser<Model>;
        using OutputType = typename BaseType::OutputType;
    public:
        template <class Iter>
        constexpr auto operator()(utils::BiIterator<Iter> args) const -> PosExpected<OutputType>
        {
            OutputType result;
            constexpr auto parser = Parser<Model>{};
            auto cmd = parser.template parse<OutputType>(result, args);
            if (!cmd)
                return glap::make_unexpected(cmd.error());
            else
                return result;
        }
    };
    template <StringLiteral Name, class... Commands>
    class Parser<model::Program<Name, Commands...>> : public Parser<Parser<model::Program<Name, Commands...>>> {
        using OutputType = model::Program<Name, Commands...>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType& program, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
    template <HasLongName CommandNames, model::IsArgument... Arguments>
    class Parser<model::Command<CommandNames, Arguments...>> : public Parser<Parser<model::Command<CommandNames, Arguments...>>> {
    public:
        using OutputType = model::Command<CommandNames, Arguments...>;
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
        {

        }
    };
    template <class ArgNames>
    class Parser<model::Flag<ArgNames>> : public Parser<Parser<model::Flag<ArgNames>>> {
        using OutputType = model::Flag<ArgNames>;
    public:
        template <class Iter>
        constexpr auto parse(OutputType&, utils::BiIterator<Iter> args) const -> PosExpected<Iter>
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