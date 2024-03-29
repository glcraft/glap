#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include "discard.h"
#include "utils.h"
#include <string_view>
#include <optional>
#endif

GLAP_EXPORT namespace glap
{
    template <auto Resolver = discard, auto Validator = discard>
    struct Value {
        using value_type = typename impl::ResolverReturnType<decltype(Resolver)>::type;
        constexpr Value() = default;
        constexpr Value(std::string_view v) : value(v)
        {}
        
        static constexpr auto resolver = Resolver;
        static constexpr auto validator = Validator;

        std::optional<value_type> value;
    };
}