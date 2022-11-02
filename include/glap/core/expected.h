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

namespace glap
{
    template <class T>
    concept IsExpected = requires(T t) {
        {t.value()} -> std::convertible_to<typename T::value_type>;
        {t.error()} -> std::convertible_to<typename T::error_type>;
    };
} // namespace glap