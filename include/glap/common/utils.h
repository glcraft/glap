#pragma once 
#include <concepts>
#include "../common/expected.h"
#include "../common/utf8.h"

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
}