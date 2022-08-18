#pragma once

#include <__concepts/same_as.h>
#include <string_view>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>
namespace glap::v2 
{
    enum class ParameterType {
        Argument,
        Flag,
        Input
    };
    template <class T>
    concept HasResultType = requires(T t) {
        typename T::result_type;
    };
    template <class T>
    concept HasNames = requires(T t) {
        std::same_as<std::remove_cvref_t<decltype(T::longname)>, std::string_view>;
        std::same_as<std::remove_cvref_t<decltype(T::shortname)>, std::optional<char32_t>>;
        // { T::longname } -> std::same_as<std::string_view>;
        // { T::shortname } -> std::same_as<std::optional<char32_t>>;
    };
    template <class T>
    concept _Argument = requires {
        std::same_as<std::remove_cvref_t<decltype(T::type)>, ParameterType>;
        T::type == ParameterType::Argument;
    };
    // template <class T>
    // concept Flag = ;
    // template <class T>
    // concept Input = ;

    template <class T>
    concept Parameter = (_Argument<T> /*|| Flag<T> || Input<T>*/) && HasNames<T> && HasResultType<T>;

    // template <std::string_view T>
    // consteval auto test() -> std::string_view {
    //     return T;
    // }

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
    
    // template <class T>
    // concept Command = HasNames<T> && HasResultType<T>;

    // template <size_t i, Parameter... P>
    // static consteval size_t _get_parameter_id(std::string_view name) noexcept {
    //         static constexpr size_t NbParams = sizeof...(P);
    //     if constexpr (i < NbParams) {
    //         using v = std::variant<P...>;
    //         using v_alt = std::variant_alternative_t<i, v>;
    //         if (v_alt::longname == name) {
    //             return i;
    //         } else {
    //             return _get_parameter_id<i + 1>(name);
    //         }
    //     } else {
    //         return -1;
    //     }
    // }
    

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

        using names = ArgNames;
        [[nodiscard]]constexpr auto resolve(std::string_view value) const requires (!std::same_as<decltype(Resolver), Discard>) {
            static_assert(std::invocable<decltype(Resolver), std::string_view>, "Resolver must be callable with std::string_view");
            return Resolver(value);
        }
    };
    
    template <class CommandNames, class... P>
    class Command {
        using Params = std::variant<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::variant_alternative_t<I, Params>;
        

        Params params;
        
        template <size_t i, StringLiteral lit>
        static consteval size_t _get_parameter_id() noexcept {
            static_assert((i < NbParams), "Parameter not found");
            if constexpr (Param<i>::names::longname == lit) {
                return i;
            } else {
                return _get_parameter<i + 1, lit>();
            }
        }
    public:
        using names = CommandNames;
        template <StringLiteral lit>
        constexpr auto& get_parameter() noexcept requires (NbParams > 0) {
            return std::get<_get_parameter_id<0, lit>()>(params);
        }
    };

    template<class... Params>
    class Parser {

    };
}