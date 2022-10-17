#pragma once 
#include <concepts>
#include <optional>
#include <string_view>
#include <algorithm>
#include "discard.h"
#include "expected.h"
#include "utf8.h"

namespace glap
{
    namespace utils {
        template <typename T, typename V>
        concept Iterable = requires(T t) {
            {*t.begin()} -> std::convertible_to<V>;
            {*t.end()} -> std::convertible_to<V>;
        };
        template <typename T, typename V>
        concept Iterator = requires(T t) {
            {*t} -> std::convertible_to<V>;
            {++t} -> std::same_as<T&>;
        };
    }
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
    namespace impl {

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
        static constexpr auto value_or_v = value_or<Value, Default>::value;

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
        static constexpr bool is_value_v = is_value<Ty, I, V>::value;

        template <class T, auto SN>
        struct OptionalValue {
            static constexpr auto value = std::optional<T>{SN};
        };
        template<class T> 
        struct OptionalValue<T, discard> {
            static constexpr auto value = std::optional<T>{};
        };
        template<> 
        struct OptionalValue<char32_t, 0> {
            static constexpr auto value = std::optional<char32_t>{};
        };
        template<auto V> 
            requires std::convertible_to<decltype(V), std::string_view> && (std::string_view(V).size() == 0)
        struct OptionalValue<std::string_view, V> {
            static constexpr auto value = std::optional<std::string_view>{};
        };
        template <class T, auto SN>
        static constexpr auto optional_value = OptionalValue<T, SN>::value;
    }

    template <StringLiteral LongName, auto ShortName = discard>
    struct Names {
        static constexpr std::string_view longname = LongName;
        static constexpr std::optional<char32_t> shortname = impl::optional_value<char32_t, ShortName>;
    };
    template <typename T>
    concept HasLongName = std::same_as<std::remove_cvref_t<decltype(T::longname)>, std::string_view>;
    template <typename T>
    concept HasNames = HasLongName<T>
        && std::same_as<std::remove_cvref_t<decltype(T::shortname)>, std::optional<char32_t>>;
    template <typename T>
    concept HasShortName = HasNames<T> && T::shortname.has_value();

    template <class ...ArgN>
    struct NameChecker 
    {
        static constexpr bool has_duplicate_longname = false;
        static constexpr bool has_duplicate_shortname = false;
    };
    template <HasNames Arg1, HasNames Arg2, class ...ArgN>
    struct NameChecker<Arg1, Arg2, ArgN...> 
    {
        static constexpr bool has_duplicate_longname = Arg1::longname == Arg2::longname || NameChecker<Arg1, ArgN...>::has_duplicate_longname || NameChecker<Arg2, ArgN...>::has_duplicate_longname;
        static constexpr bool has_duplicate_shortname = Arg1::shortname.has_value() && Arg2::shortname.has_value() && Arg1::shortname.value() == Arg2::shortname.value() || NameChecker<Arg1, ArgN...>::has_duplicate_shortname || NameChecker<Arg2, ArgN...>::has_duplicate_shortname;
    };
    template <typename Arg1, typename Arg2, class ...ArgN>
        requires HasNames<Arg1> && (!HasNames<Arg2>)
    struct NameChecker<Arg1, Arg2, ArgN...>
    {
        static constexpr bool has_duplicate_longname = NameChecker<Arg1, ArgN...>::has_duplicate_longname;
        static constexpr bool has_duplicate_shortname = NameChecker<Arg1, ArgN...>::has_duplicate_shortname;
    };
    template <typename Arg1, class ...ArgN>
    struct NameChecker<Arg1, Arg1, ArgN...>
    {
        static_assert(!std::is_same_v<Arg1, Arg1>, "Duplicate parameter");
    };
    template <auto Resolver = discard, auto Validator = discard>
    struct Value {
        constexpr Value() = default;
        constexpr Value(std::string_view v) : value(v)
        {}
        
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;

        std::optional<std::string_view> value;

        [[nodiscard]]constexpr auto resolve() const requires (!std::same_as<decltype(Resolver), Discard>) {
            static_assert(std::invocable<decltype(Resolver), std::string_view>, "Resolver must be callable with std::string_view");
            return value ? Resolver(value.value()) : std::optional<decltype(Resolver(std::string_view{}))>{};
        }
        [[nodiscard]]constexpr auto validate() const requires (!std::same_as<decltype(Validator), Discard>) {
            static_assert(std::invocable<decltype(Validator), std::string_view>, "Validator must be callable with std::string_view");
            return value ? Validator(value.value()) : false;
        }
    };
}