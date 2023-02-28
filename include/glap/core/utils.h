#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include <concepts>
#include <optional>
#include <string_view>
#include <algorithm>
#include "discard.h"
#include "expected.h"
#include "utf8.h"
#include "convertible_to.h"
#endif

GLAP_EXPORT namespace glap
{
    namespace impl {
        template <typename T, typename V>
        concept Range = requires(T t) {
            {*t.begin()} -> impl::convertible_to<V>;
            {*t.end()} -> impl::convertible_to<V>;
        };
        template <typename T, typename V>
        concept Iterator = requires(T t) {
            {*t} -> impl::convertible_to<V>;
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
}

namespace glap::impl {
    template <template<class> class Predicate, class Type>
    concept IsMetaPredicate =  requires {
        {Predicate<Type>::value} -> std::convertible_to<bool>;
    };

    template <template<class> class Predicate, class DefaultType, class CurrentType, class ...Ts>
        requires IsMetaPredicate<Predicate, CurrentType>
    struct TypeSelectorTrait {
        using Type = typename std::conditional<Predicate<CurrentType>::value, CurrentType, typename TypeSelectorTrait<Predicate, DefaultType, Ts...>::type>::type;
    };
    template <template<class> class Predicate, class DefaultType, class CurrentType>
        requires IsMetaPredicate<Predicate, CurrentType>
    struct TypeSelectorTrait<Predicate, DefaultType, CurrentType> {
        using Type = typename std::conditional<Predicate<CurrentType>::value, CurrentType, DefaultType>::type;
    };
    template <template<class> class Predicate, class DefaultType, class CurrentType, class ...Ts>
        requires IsMetaPredicate<Predicate, CurrentType>
    using TypeSelector = typename TypeSelectorTrait<Predicate, DefaultType, CurrentType, Ts...>::Type;

    template <auto Value, auto Default>
    struct ValueOrTrait {
        static constexpr auto value = Value;
    };
    template <auto Default>
    struct ValueOrTrait<glap::discard, Default> {
        static constexpr auto value = Default;
    };
    template <auto Value, auto Default>
    inline constexpr auto ValueOr = ValueOrTrait<Value, Default>::value;

    template <auto I, auto V>
    struct IsEqualTrait
    {
        static constexpr bool value = false;
    };
    template <auto V>
    struct IsEqualTrait<V, V>
    {
        static constexpr bool value = true;
    };
    template <auto I, auto V>
    inline constexpr bool IsEqual = IsEqualTrait<I, V>::value;

    template <class T, auto SN>
    struct MetaOptionalTrait {
        static constexpr auto value = std::optional<T>{SN};
    };
    template<class T>
    struct MetaOptionalTrait<T, discard> {
        static constexpr auto value = std::optional<T>{};
    };
    template<>
    struct MetaOptionalTrait<char32_t, 0> {
        static constexpr auto value = std::optional<char32_t>{};
    };
    template<auto V>
        requires impl::convertible_to<decltype(V), std::string_view> && (std::string_view(V).size() == 0)
    struct MetaOptionalTrait<std::string_view, V> {
        static constexpr auto value = std::optional<std::string_view>{};
    };
    template <class T, auto SN>
    inline constexpr auto MetaOptional = MetaOptionalTrait<T, SN>::value;
}

GLAP_EXPORT namespace glap {
    template <StringLiteral Name, auto ShortName = discard>
    struct Names {
        static constexpr std::string_view name = Name;
        static constexpr std::optional<char32_t> shortname = impl::MetaOptional<char32_t, ShortName>;
    };
    template <typename T>
    concept HasLongName = std::same_as<std::remove_cvref_t<decltype(T::name)>, std::string_view>;
    template <typename T>
    concept HasNames = HasLongName<T>
        && std::same_as<std::remove_cvref_t<decltype(T::shortname)>, std::optional<char32_t>>;
    template <typename T>
    concept HasShortName = HasNames<T> && T::shortname.has_value();
    template <typename T>
    concept IsResolver = std::invocable<T, std::string_view>;
    template <typename T>
    concept IsValidator = std::invocable<T, std::string_view> && std::same_as<std::invoke_result_t<T, std::string_view>, bool>;
}

namespace glap::impl
{
    template <class ...ArgN>
    struct NameChecker
    {
        static constexpr bool has_duplicate_longname = false;
        static constexpr bool has_duplicate_shortname = false;
    };
    template <HasNames Arg1, HasNames Arg2, class ...ArgN>
    struct NameChecker<Arg1, Arg2, ArgN...>
    {
        static constexpr bool has_duplicate_longname = Arg1::name == Arg2::name || NameChecker<Arg1, ArgN...>::has_duplicate_longname || NameChecker<Arg2, ArgN...>::has_duplicate_longname;
        static constexpr bool has_duplicate_shortname = (Arg1::shortname.has_value() && Arg2::shortname.has_value() && Arg1::shortname.value() == Arg2::shortname.value()) || NameChecker<Arg1, ArgN...>::has_duplicate_shortname || NameChecker<Arg2, ArgN...>::has_duplicate_shortname;
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
        static_assert(!std::is_same_v<Arg1, Arg1>, "Duplicate argument");
    };
    template <class T>
    struct ResolverReturnTrait
    {
        using Type = T;
    };
    template <class D>
        requires std::same_as<std::remove_cv_t<D>, Discard>
    struct ResolverReturnTrait<D>
    {
        using Type = std::string_view;
    };
    template <IsResolver T>
    struct ResolverReturnTrait<T>
    {
        using Type = typename ResolverReturnTrait<std::invoke_result_t<T, std::string_view>>::Type;
    };
    template <class T, class D>
        requires std::same_as<std::remove_cv_t<D>, Discard>
    struct ResolverReturnTrait<glap::expected<T, D>>
    {
        using Type = T;
    };
    template <class T>
    using ResolverReturn = typename ResolverReturnTrait<T>::Type;
}