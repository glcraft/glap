#pragma once
#if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
#include <concepts>
#else
#include <type_traits>
#endif


namespace glap
{
    namespace impl {
    #if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
        template <typename From, typename To>
        concept convertible_to = std::convertible_to<From, To>;
    #else
    template <class From, class To>
    concept convertible to =
        std::is_convertible_v<From, To> &&
        requires {
            static_cast<To>(std::declval<From>());
        };
    #endif
    }
}