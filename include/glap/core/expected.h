#pragma once
#ifndef GLAP_USE_TL_EXPECTED 
#   ifndef __cpp_lib_expected
#       error "standard expected requires C++23"
#   endif

#include <expected>
namespace glap
{
    template <class T, class E>
    using expected = std::expected<T, E>;
    template <class E>
    using unexpected = std::unexpected<E>;
} // namespace glap
#else
#include <tl/expected.hpp>
namespace glap
{
    template <class T, class E>
    using expected = tl::expected<T, E>;
    template <class E>
    using unexpected = tl::unexpected<E>;
} // namespace glap
#endif

#if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
#include <concepts>
#else
#include <type_traits>
#endif


namespace glap
{
    namespace details {
    #if defined(__cpp_lib_concepts) && __cpp_lib_concepts >= 201806L
        template <typename T, typename U>
        concept convertible_to = std::convertible_to<T, U>;
    #else
        template <typename T, typename U>
        concept convertible_to = std::is_convertible<U, T>;
    #endif
    }

    template <class T>
    concept IsExpected = requires(T t) {

        {t.value()} -> convertible_to<typename T::value_type>;
        {t.error()} -> convertible_to<typename T::error_type>;
    };
} // namespace glap