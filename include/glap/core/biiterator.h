#pragma once
#include "utils.h"
#include <string_view>
namespace glap::impl 
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