#pragma once

#include <tuple>
#include <string_view>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>
namespace glap::v2 
{
    enum class ParameterType {
        Argument,
        Flag,
        Input
    };

    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
        constexpr operator std::string_view() const {
            return std::string_view(value, N - 1);
        }
        
        char value[N];
    };

    struct Discard {};
    static constexpr Discard discard = {};
    template <StringLiteral LongName, auto ShortName = discard>
    struct Names {
        static constexpr std::string_view longname = LongName.value;
        static constexpr std::optional<char32_t> shortname = std::same_as<decltype(ShortName), Discard> ? std::optional<char32_t>{} : std::optional<char32_t>{ShortName};
    };
    template <class CRTP>
        requires requires {
            std::same_as<std::remove_cvref_t<decltype(CRTP::longname)>, std::string_view>;
            std::same_as<std::remove_cvref_t<decltype(CRTP::shortname)>, std::optional<char32_t>>;
        }
    class GetNames {
    public:
        constexpr auto longname() const noexcept {
            return CRTP::longname;
        }
        constexpr auto shortname() const noexcept {
            return CRTP::hortname;
        }
    };

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    class Argument : public GetNames<ArgNames> {
    public:
        std::string_view value;

        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Argument;
        using names = ArgNames;

        [[nodiscard]]constexpr auto resolve() const requires (!std::same_as<decltype(Resolver), Discard>) {
            static_assert(std::invocable<decltype(Resolver), std::string_view>, "Resolver must be callable with std::string_view");
            return Resolver(value);
        }
    };
    template <class ArgNames>
    class Flag : public GetNames<ArgNames> {
    public:
        size_t occurences = 0;

        static constexpr auto type = ParameterType::Flag;
        using names = ArgNames;
    };
    template <auto Resolver = discard, auto Validator = discard>
    class Inputs {
    public:
        std::vector<std::string_view> values;

        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Input;
    };

    template <class T>
    concept Parameter = requires {
        std::same_as<std::remove_cvref_t<decltype(T::type)>, ParameterType>;
    };
    
    template <class CommandNames, Parameter... P>
    class Command : public GetNames<CommandNames> {
        using Params = std::tuple<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;
        

        Params params;
        
        template <size_t i, StringLiteral lit>
        static consteval size_t _get_parameter_id() noexcept {
            static_assert((i < NbParams), "Parameter not found");
            if constexpr (Param<i>::names::longname == lit) {
                return i;
            } else {
                return _get_parameter_id<i + 1, lit>();
            }
        }
    public:
        using names = CommandNames;
        template <StringLiteral lit>
        constexpr auto& get_parameter() noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
        template <StringLiteral lit>
        constexpr const auto& get_parameter() const noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
    };

    template<class... Params>
    class Parser {

    };
}