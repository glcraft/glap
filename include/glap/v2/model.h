#pragma once
#include "discard.h"
#include "utils.h"
#include "container.h"
#include <variant>
namespace glap::v2::model
{
    enum class ParameterType {
        Argument,
        Flag,
        Input
    };

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Argument : public ArgNames, public Value<Resolver, Validator> {
        static constexpr auto type = ParameterType::Argument;
    };
    
    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Arguments : public ArgNames, public Container<Argument<ArgNames, Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Argument;
    };
    template <class ArgNames>
    struct Flag : public ArgNames {
        size_t occurences = 0;
        static constexpr auto type = ParameterType::Flag;
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Input : public Value<Resolver, Validator> {
        static constexpr auto type = ParameterType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    struct Inputs : public Container<Input<Resolver, Validator>, N> {
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Input;
    };

    template <class T>
    concept IsParameter = requires {
        std::same_as<std::remove_cvref_t<decltype(T::type)>, ParameterType>;
    };
    
    template <class CommandNames, IsParameter... P>
    class Command : public CommandNames {
        using NameCheck = NameChecker<P...>;
        static_assert(!NameCheck::has_duplicate_longname, "parameters has duplicate long name");
        static_assert(!NameCheck::has_duplicate_shortname, "parameters has duplicate short name");

        using Params = std::tuple<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;
        
        template <size_t i, StringLiteral lit>
        static consteval size_t _get_parameter_id() noexcept {
            static_assert((i < NbParams), "Parameter not found");
            if constexpr (Param<i>::Longname == lit) {
                return i;
            } else {
                return _get_parameter_id<i + 1, lit>();
            }
        }
    public:
        Params params;
        template <StringLiteral lit>
        constexpr auto& get_parameter() noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
        template <StringLiteral lit>
        constexpr const auto& get_parameter() const noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
    };
    template<class... Commands>
    struct Program {
        std::string_view program;
        std::variant<Commands...> command;
    };
}