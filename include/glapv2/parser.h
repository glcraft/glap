#pragma once

#include <algorithm>
#include <cstddef>
#include <tuple>
#include <string_view>
#include <concepts>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>
namespace glap::v2 
{
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

    template <auto Value, size_t Default>
    struct value_or {
        static constexpr auto value = Default;
    };
    template <auto Value, size_t Default>
        requires std::convertible_to<decltype(Value), size_t>
    struct value_or<Value, Default> {
        static constexpr size_t value = Value;
    };
    template <auto Value, size_t Default>
    constexpr auto value_or_v = value_or<Value, Default>::value;

    template <class Ty, auto I, size_t V>
    struct is_value
    {
        static constexpr bool value = false;
    };
    template <class Ty, size_t V>
    struct is_value<Ty, V, V>
    {
        static constexpr bool value = true;
    };
    template <class Ty, auto I, size_t V>
    constexpr bool is_value_v = is_value<Ty, I, V>::value;

    template <class T, auto N = discard>
    class Container {
    public: 
        using value_type = T;
    private:
        using n_type = std::remove_cvref_t<decltype(N)>;
        static constexpr auto is_n_discard = std::is_same_v<n_type, Discard>;
        static constexpr auto is_n_zero = is_value_v<n_type, N, 0>;
        using dynamic_vector = std::vector<value_type>;
        using fixed_vector = FixedVector<value_type, value_or_v<N, 0>>;
    public:
        using container_type = std::conditional_t<is_n_discard || is_n_zero, dynamic_vector, fixed_vector>;
        container_type values;

        constexpr auto size() const noexcept {
            return values.size();
        }

        [[nodiscard]]constexpr const auto& get(size_t i) const noexcept {
            return this->values[i];
        }
        [[nodiscard]]constexpr auto& get(size_t i) noexcept {
            return this->values[i];
        }
        template <size_t I>
        [[nodiscard]]constexpr auto& get() noexcept requires (I < value_or<N, std::numeric_limits<size_t>::max()>::value) {
            return this->values[I];
        }
        template <size_t I>
        [[nodiscard]]constexpr const auto& get() const noexcept requires (I < value_or<N, std::numeric_limits<size_t>::max()>::value) {
            return this->values[I];
        }
    };

    template <StringLiteral LongName, auto ShortName = discard>
    struct Names {
    private:
        template <auto SN>
        struct _short_name {
            static constexpr auto value = std::optional<char32_t>{SN};
        };
        template<> struct _short_name<discard> {
            static constexpr auto value = std::optional<char32_t>{};
        };
        template<> struct _short_name<0> {
            static constexpr auto value = std::optional<char32_t>{};
        };
    public:
        static constexpr std::string_view Longname = LongName;
        static constexpr std::optional<char32_t> Shortname = _short_name<ShortName>::value;

        constexpr auto longname() const noexcept {
            return Longname;
        }
        constexpr auto shortname() const noexcept {
            return Shortname;
        }
    };
    template <typename T>
    concept HasNames = requires {
        std::same_as<std::remove_cvref_t<decltype(T::Longname)>, std::string_view>;
        std::same_as<std::remove_cvref_t<decltype(T::Shortname)>, std::optional<char32_t>>;
    };
   
    template <class ...ArgN>
    struct NameChecker 
    {};
    template <HasNames Arg1, HasNames Arg2, class ...ArgN>
    struct NameChecker<Arg1, Arg2, ArgN...> : NameChecker<Arg1, ArgN...>, NameChecker<Arg2, ArgN...> {
        static_assert(Arg1::Longname != Arg2::Longname, "Duplicate longname");
        static_assert(!Arg1::Shortname.has_value() || !Arg2::Shortname.has_value() || Arg1::Shortname.value() != Arg2::Shortname.value(), "Duplicate shortname");
    };
    template <typename Arg1, typename Arg2, class ...ArgN>
        requires HasNames<Arg1> && (!HasNames<Arg2>)
    struct NameChecker<Arg1, Arg2, ArgN...> : NameChecker<Arg1, ArgN...> 
    {};
    template <typename Arg1, class ...ArgN>
    struct NameChecker<Arg1, Arg1, ArgN...>
    {
        static_assert(!std::is_same_v<Arg1, Arg1>, "Duplicate parameter");
    };

    enum class ParameterType {
        Argument,
        Flag,
        Input
    };

    template <class ArgNames, auto Resolver = discard, auto Validator = discard>
    struct Argument : public ArgNames {
        std::string_view value;

        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Argument;

        [[nodiscard]]constexpr auto resolve() const requires (!std::same_as<decltype(Resolver), Discard>) {
            static_assert(std::invocable<decltype(Resolver), std::string_view>, "Resolver must be callable with std::string_view");
            return Resolver(value);
        }
        [[nodiscard]]constexpr auto validate() const requires (!std::same_as<decltype(Validator), Discard>) {
            static_assert(std::invocable<decltype(Validator), std::string_view>, "Validator must be callable with std::string_view");
            return Validator(value);
        }
    };
    
    template <class ArgNames, auto N = discard, auto Resolver = discard, auto Validator = discard>
    class Arguments : public ArgNames, public Container<Argument<ArgNames, Resolver, Validator>, N> {
    public:
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Argument;
    };
    
    template <class ArgNames>
    class Flag : public ArgNames {
    public:
        size_t occurences = 0;

        static constexpr auto type = ParameterType::Flag;
    };
    template <auto Resolver = discard, auto Validator = discard>
    class Input {
    public:
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Input;
    };
    template <auto N = discard, auto Resolver = discard, auto Validator = discard>
    class Inputs : public Container<Input<Resolver, Validator>, N> {
    public:
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;
        static constexpr auto type = ParameterType::Input;
    };

    template <class T>
    concept Parameter = requires {
        std::same_as<std::remove_cvref_t<decltype(T::type)>, ParameterType>;
    };
    
    template <class CommandNames, Parameter... P>
    class Command : public CommandNames, public NameChecker<P...> {
        using Params = std::tuple<P...>;
        static constexpr size_t NbParams = sizeof...(P);
        template <size_t I>
        using Param = std::tuple_element_t<I, Params>;
        

        Params params;
        
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