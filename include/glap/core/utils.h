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
    } or requires {
        {Predicate<Type>::VALUE} -> std::convertible_to<bool>;
    };

    template <template<class> class Predicate, class DefaultType, class CurrentType, class ...Ts>
        requires IsMetaPredicate<Predicate, CurrentType>
    struct TypeSelectorTrait {
        using Type = typename std::conditional<Predicate<CurrentType>::value, CurrentType, typename TypeSelectorTrait<Predicate, DefaultType, Ts...>::Type>::type;
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
        static constexpr auto VALUE = Value;
    };
    template <auto Default>
    struct ValueOrTrait<glap::discard, Default> {
        static constexpr auto VALUE = Default;
    };
    template <auto Value, auto Default>
    inline constexpr auto ValueOr = ValueOrTrait<Value, Default>::VALUE;

    template <auto I, auto V>
    struct IsEqualTrait
    {
        static constexpr bool VALUE = false;
    };
    template <auto V>
    struct IsEqualTrait<V, V>
    {
        static constexpr bool VALUE = true;
    };
    template <auto I, auto V>
    inline constexpr bool IsEqual = IsEqualTrait<I, V>::VALUE;

    template <class T, auto SN>
    struct MetaOptionalTrait {
        static constexpr auto VALUE = std::optional<T>{SN};
    };
    template<class T>
    struct MetaOptionalTrait<T, discard> {
        static constexpr auto VALUE = std::optional<T>{};
    };
    template<>
    struct MetaOptionalTrait<char32_t, 0> {
        static constexpr auto VALUE = std::optional<char32_t>{};
    };
    template<auto V>
        requires impl::convertible_to<decltype(V), std::string_view> && (std::string_view(V).size() == 0)
    struct MetaOptionalTrait<std::string_view, V> {
        static constexpr auto VALUE = std::optional<std::string_view>{};
    };
    template <class T, auto SN>
    inline constexpr auto MetaOptional = MetaOptionalTrait<T, SN>::VALUE;
}

GLAP_EXPORT namespace glap {
    template <StringLiteral Name, auto ShortName = discard>
    struct Names {
        static constexpr std::string_view NAME = Name;
        static constexpr std::optional<char32_t> SHORTNAME = impl::MetaOptional<char32_t, ShortName>;
    };
    template <typename T>
    concept HasLongName = std::same_as<std::remove_cvref_t<decltype(T::NAME)>, std::string_view>;
    template <typename T>
    concept HasNames = HasLongName<T>
        && std::same_as<std::remove_cvref_t<decltype(T::SHORTNAME)>, std::optional<char32_t>>;
    template <typename T>
    concept HasShortName = HasNames<T> && T::SHORTNAME.has_value();
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
        static constexpr bool HAS_DUPLICATE_LONGNAME = false;
        static constexpr bool HAS_DUPLICATE_SHORTNAME = false;
    };
    template <HasNames Arg1, HasNames Arg2, class ...ArgN>
    struct NameChecker<Arg1, Arg2, ArgN...>
    {
        static constexpr bool HAS_DUPLICATE_LONGNAME = Arg1::NAME == Arg2::NAME || NameChecker<Arg1, ArgN...>::HAS_DUPLICATE_LONGNAME || NameChecker<Arg2, ArgN...>::HAS_DUPLICATE_LONGNAME;
        static constexpr bool HAS_DUPLICATE_SHORTNAME = (Arg1::SHORTNAME.has_value() && Arg2::SHORTNAME.has_value() && Arg1::SHORTNAME.value() == Arg2::SHORTNAME.value()) || NameChecker<Arg1, ArgN...>::HAS_DUPLICATE_SHORTNAME || NameChecker<Arg2, ArgN...>::HAS_DUPLICATE_SHORTNAME;
    };
    template <typename Arg1, typename Arg2, class ...ArgN>
        requires HasNames<Arg1> && (!HasNames<Arg2>)
    struct NameChecker<Arg1, Arg2, ArgN...>
    {
        static constexpr bool HAS_DUPLICATE_LONGNAME = NameChecker<Arg1, ArgN...>::HAS_DUPLICATE_LONGNAME;
        static constexpr bool HAS_DUPLICATE_SHORTNAME = NameChecker<Arg1, ArgN...>::HAS_DUPLICATE_SHORTNAME;
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