#pragma once

#ifndef GLAP_MODULE
#include "core/base.h"
#include "core/discard.h"
#include "core/utils.h"
#include "core/container.h"
#include "core/value.h"
#include <variant>
#endif

GLAP_EXPORT namespace glap::model
{
    enum class ArgumentType {
        Parameter,
        Flag,
        Input
    };
    template <class Command>
    struct DefaultCommand : public Command {
        using command_t = Command;
    };
    namespace impl {
        template <class T>
        struct IsDefaultCommand : std::false_type {};
        template <class T>
        struct IsDefaultCommand<DefaultCommand<T>> : std::true_type {};
    }
    template <class T>
    struct GetCommand {
        using CommandType = T;
    };
    template <class T>
    struct GetCommand<DefaultCommand<T>> {
        using CommandType = T;
    };

    template <class T, ArgumentType PType>
    concept IsArgumentTyped = requires {
        T::type;
    } && (T::type == PType);

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Parameter : ArgNames, Value<Resolver, Validator> {
        using value_type = typename glap::impl::ResolverReturnType<decltype(Resolver)>::type;
        constexpr Parameter() = default;
        constexpr Parameter(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Parameter;
    };

    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Parameters : ArgNames, Container<Parameter<ArgNames, Resolver, Validator>, N> {
        using value_type = typename glap::impl::ResolverReturnType<decltype(Resolver)>::type;
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ArgumentType::Parameter;
    };
    template <class ArgNames>
    struct Flag : ArgNames {
        size_t occurences = 0;
        static constexpr auto type = ArgumentType::Flag;
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Input : Value<Resolver, Validator> {
        using value_type = typename glap::impl::ResolverReturnType<decltype(Resolver)>::type;
        constexpr Input() = default;
        constexpr Input(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Inputs : Container<Input<Resolver, Validator>, N> {
        using value_type = typename glap::impl::ResolverReturnType<decltype(Resolver)>::type;
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ArgumentType::Input;
    };

    template <class T>
    concept IsArgument = std::same_as<std::remove_cvref_t<decltype(T::type)>, ArgumentType>;

    template <class CommandNames, IsArgument... Arguments>
    struct Command : CommandNames {
        using Params = std::tuple<Arguments...>;
        Params arguments;
    private:
        using NameCheck = glap::impl::NameChecker<Arguments...>;
        static_assert(!NameCheck::has_duplicate_longname, "arguments has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "arguments has duplicate short name");


        static constexpr size_t NbParams = sizeof...(Arguments);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;

        template <size_t i, StringLiteral lit>
        static consteval size_t _get_argument_id() noexcept {
            static_assert((i < NbParams), "Argument not found");
            if constexpr (Param<i>::name == lit) {
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
        template <StringLiteral lit>
        constexpr const auto& get_argument() const noexcept requires (NbParams > 0) {
            return std::get<_get_argument_id<0, lit>()>(arguments);
        }
        constexpr const auto& get_inputs() const noexcept requires (NbParams > 0 && (IsArgumentTyped<Arguments, ArgumentType::Input> || ...)) {
            return std::get<_get_input_id<0>()>(arguments);
        }

    };
    template<StringLiteral Name, class... Commands>
    struct Program {
        using NameCheck = glap::impl::NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "arguments has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "arguments has duplicate short name");

        static constexpr std::string_view name = Name;
        using default_command_t = typename glap::impl::TypeSelectorTrait<impl::IsDefaultCommand, Discard, Commands...>::type;
        std::string_view program;
        std::variant<typename GetCommand<Commands>::type...> command;
    };
}