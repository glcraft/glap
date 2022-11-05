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
    enum class DefaultCommand {
        FirstDefined,
        None
    };

    template <class T, ArgumentType PType>
    concept IsArgumentTyped = requires {
        T::type;
    } && (T::type == PType);

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Parameter : public ArgNames, public Value<Resolver, Validator> {
        using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
        constexpr Parameter() = default;
        constexpr Parameter(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Parameter;
    };
    
    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Parameters : public ArgNames, public Container<Parameter<ArgNames, Resolver, Validator>, N> {
        using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
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
        using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
        constexpr Input() = default;
        constexpr Input(std::string_view v) : Value<Resolver, Validator>(v)
        {}
        static constexpr auto type = ArgumentType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Inputs : public Container<Input<Resolver, Validator>, N> {
        using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ArgumentType::Input;
    };

    template <class T>
    concept IsArgument = std::same_as<std::remove_cvref_t<decltype(T::type)>, ArgumentType>;
    
    template <class CommandNames, IsArgument... Arguments>
    struct Command : public CommandNames {
        using Params = std::tuple<Arguments...>;
        Params arguments;
    private:
        using NameCheck = NameChecker<Arguments...>;
        static_assert(!NameCheck::has_duplicate_longname, "arguments has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "arguments has duplicate short name");

        
        static constexpr size_t NbParams = sizeof...(Arguments);
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
        template <StringLiteral lit>
        constexpr const auto& get_argument() const noexcept requires (NbParams > 0) {
            return std::get<_get_argument_id<0, lit>()>(arguments);
        }
        constexpr const auto& get_inputs() const noexcept requires (NbParams > 0 && (IsArgumentTyped<Arguments, ArgumentType::Input> || ...)) {
            return std::get<_get_input_id<0>()>(arguments);
        }

    };
    template<StringLiteral Name, DefaultCommand def_cmd, class... Commands>
    struct Program {
        using NameCheck = NameChecker<Commands...>;
        static_assert(!NameCheck::has_duplicate_longname, "arguments has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "arguments has duplicate short name");
        
        static constexpr std::string_view name = Name;
        static constexpr auto default_command = def_cmd;
        std::string_view program;
        std::variant<Commands...> command;
    };
}