#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include "convertible_to.h"
#include <version>
#endif

#ifndef GLAP_USE_TL_EXPECTED
#   ifndef __cpp_lib_expected
#       error "standard expected requires C++23"
#   endif

#include <expected>

GLAP_EXPORT namespace glap
{
    template <class T, class E>
    using expected = std::expected<T, E>;
    template <class E>
    using unexpected = std::unexpected<E>;
} // namespace glap
#else

#include <tl/expected.hpp>

GLAP_EXPORT namespace glap
{
    template <class T, class E>
    using expected = tl::expected<T, E>;
    template <class E>
    using unexpected = tl::unexpected<E>;
} // namespace glap
#endif

GLAP_EXPORT namespace glap
{
    template <class T>
    concept IsExpected = requires(T t) {
        {t.value()} -> impl::convertible_to<typename T::value_type>;
        {t.error()} -> impl::convertible_to<typename T::error_type>;
    };
} // namespace glap