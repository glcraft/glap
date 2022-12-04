#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include "utils.h"
#include <string_view>
#endif

GLAP_EXPORT namespace glap::impl
{
    template <Iterator<std::string_view> T>
    struct BiIterator {
        BiIterator(T begin, T end) : begin(begin), end(end)
        {}
        BiIterator(const BiIterator&) = default;
        BiIterator(BiIterator&&) = default;
        T begin;
        T end;

        size_t size() const {
            return std::distance(begin, end);
        }
    };
    template <class T>
    BiIterator(T,T) -> BiIterator<T>;
}