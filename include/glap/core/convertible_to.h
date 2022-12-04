#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include <version>
#if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
#include <concepts>
#else
#include <type_traits>
#endif
#endif

GLAP_EXPORT namespace glap
{
    namespace impl {
    #if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
        template <typename T, typename U>
        concept convertible_to = std::convertible_to<T, U>;
    #else
        template <typename T, typename U>
        concept convertible_to = std::is_convertible_v<U, T>;
    #endif
    }
}