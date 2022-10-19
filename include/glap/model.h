#pragma once
#include "core/discard.h"
#include "core/utils.h"
#include "core/container.h"
#include <variant>
namespace glap::model
{
    enum class ArgumentType {
        Parameter,
        Flag,
        Input
    };

    template <class T, ArgumentType PType>
    concept IsArgumentTyped = requires {
        T::type;
    } && (T::type == PType);

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Parameter : public ArgNames, public Value<Resolver, Validator> {
        constexpr Parameter() = default;
        constexpr Parameter(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Parameter;
    };
    
    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Parameters : public ArgNames, public Container<Parameter<ArgNames, Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ArgumentType::Parameter;
    };
    template <class ArgNames>
    struct Flag : public ArgNames {
        size_t occurences = 0;
        static constexpr auto type = ArgumentType::Flag;
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Input : public Value<Resolver, Validator> {
        constexpr Input() = default;
        constexpr Input(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Inputs : public Container<Input<Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ArgumentType::Input;
    };

    template <class T>
    concept IsArgument = std::same_as<std::remove_cvref_t<decltype(T::type)>, ArgumentType>;
    
    template <class CommandNames, IsArgument... P>
    class Command : public CommandNames {
        using NameCheck = NameChecker<P...>;
        static_assert(!NameCheck::has_duplicate_longname, "arguments has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "arguments has duplicate short name");

        using Params = std::tuple<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;
        
        template <size_t i, StringLiteral lit>
        static consteval size_t _get_argument_id() noexcept {
            static_assert((i < NbParams), "Argument not found");
            if constexpr (Param<i>::longname == lit) {
                return i;
            } else {
                return _get_argument_id<i + 1, lit>();
            }
        }
        template <size_t i>
        static consteval size_t _get_input_id() noexcept {
            static_assert((i < NbParams), "No input in command arguments");
            if constexpr (Param<i>::type == ArgumentType::Input) {
                return i;
            } else {
                return _get_input_id<i + 1>();
            }
        }
    public:
        Params params;
        template <StringLiteral lit>
        constexpr auto& get_argument() noexcept requires (NbParams > 0) {
            return std::get<_get_argument_id<0, lit>()>(params);
        }
        template <StringLiteral lit>
        constexpr const auto& get_argument() const noexcept requires (NbParams > 0) {
            return std::get<_get_argument_id<0, lit>()>(params);
        }
        constexpr const auto& get_inputs() const noexcept requires (NbParams > 0) {
            return std::get<_get_input_id<0>()>(params);
        }

    };
    template<class... Commands>
    struct Program {
        std::string_view program;
        std::variant<Commands...> command;
    };
}