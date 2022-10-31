#pragma once
#ifdef __cpp_lib_expected
#include <expected>
template <class T, class E>
using expected = std::expected<T, E>;
template <class E>
using unexpected = std::unexpected<E>;
#else
#include <tl/expected.hpp>
namespace glap
{
    template <class T, class E>
    using expected = tl::expected<T, E>;
    template <class E>
    using unexpected = tl::unexpected<E>;
    template <class T>
    concept IsExpected = requires(T t) {
        {t.value()} -> std::convertible_to<typename T::value_type>;
        {t.error()} -> std::convertible_to<typename T::error_type>;
    };
}
#endif