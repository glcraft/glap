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
    enum class ArgumentKind {
        Parameter,
        Flag,
        Input
    };
    template <class Command>
    struct DefaultCommand : public Command {
        using CommandType = Command;
    };
    namespace impl {
        template <class T>
        struct IsDefaultCommand : std::false_type {};
        template <class T>
        struct IsDefaultCommand<DefaultCommand<T>> : std::true_type {};
    }
    template <class T>
    struct GetCommand {
        using Type = T;
    };
    template <class T>
    struct GetCommand<DefaultCommand<T>> {
        using Type = T;
    };

    template <class T, ArgumentKind PType>
    concept IsArgumentTyped = requires { T::KIND; } && (T::KIND == PType);

    template <class ArgNames, auto Resolver = DISCARD, auto Validator = DISCARD>
    struct Parameter : ArgNames, Value<Resolver, Validator> {
        using ValueType = typename glap::impl::ResolverReturn<decltype(Resolver)>;
        constexpr Parameter() = default;
        constexpr Parameter(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto KIND = ArgumentKind::Parameter;
    };

    template <class ArgNames, auto N = DISCARD, auto Resolver = DISCARD, auto Validator = DISCARD>
    struct Parameters : ArgNames, Container<Parameter<ArgNames, Resolver, Validator>, N> {
        using ValueType = typename glap::impl::ResolverReturn<decltype(Resolver)>;
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto KIND = ArgumentKind::Parameter;
    };
    template <class ArgNames>
    struct Flag : ArgNames {
        size_t occurences = 0;
        static constexpr auto KIND = ArgumentKind::Flag;
    };
    template <auto Resolver = DISCARD, auto Validator = DISCARD>
    struct Input : Value<Resolver, Validator> {
        using ValueType = typename glap::impl::ResolverReturn<decltype(Resolver)>;
        constexpr Input() = default;
        constexpr Input(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto KIND = ArgumentKind::Input;
    };
    template <auto N = DISCARD, auto Resolver = DISCARD, auto Validator = DISCARD>
    struct Inputs : Container<Input<Resolver, Validator>, N> {
        using ValueType = typename glap::impl::ResolverReturn<decltype(Resolver)>;
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto KIND = ArgumentKind::Input;
    };

    template <class T>
    concept IsArgument = std::same_as<std::remove_cvref_t<decltype(T::KIND)>, ArgumentKind>;

    template <class CommandNames, IsArgument... Arguments>
    struct Command : CommandNames {
        using Params = std::tuple<Arguments...>;
        Params arguments;
    private:
        using NameCheck = glap::impl::NameChecker<Arguments...>;
        static_assert(!NameCheck::HAS_DUPLICATE_LONGNAME, "arguments has duplicate long name");
        static_assert(!NameCheck::HAS_DUPLICATE_SHORTNAME, "arguments has duplicate short name");


        static constexpr size_t NbParams = sizeof...(Arguments);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;

        template <size_t i, StringLiteral lit>
        static consteval size_t _get_argument_id() noexcept {
            static_assert((i < NbParams), "Argument not found");
            if constexpr (Param<i>::NAME == lit) {
                return i;
            } else {
                return _get_argument_id<i + 1, lit>();
            }
        }
        template <size_t i>
        static consteval size_t _get_input_id() noexcept {
            static_assert((i < NbParams), "No input in command arguments");
            if constexpr (Param<i>::KIND == ArgumentKind::Input) {
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
        constexpr const auto& get_inputs() const noexcept requires (NbParams > 0 && (IsArgumentTyped<Arguments, ArgumentKind::Input> || ...)) {
            return std::get<_get_input_id<0>()>(arguments);
        }

    };
    template<StringLiteral Name, class... Commands>
    struct Program {
        using NameCheck = glap::impl::NameChecker<Commands...>;
        static_assert(!NameCheck::HAS_DUPLICATE_LONGNAME, "arguments has duplicate long name");
        static_assert(!NameCheck::HAS_DUPLICATE_SHORTNAME, "arguments has duplicate short name");

        static constexpr std::string_view NAME = Name;
        using DefaultCommandType = typename glap::impl::TypeSelectorTrait<impl::IsDefaultCommand, Discard, Commands...>::Type;
        std::string_view program;
        std::variant<typename GetCommand<Commands>::Type...> command;
    };
}