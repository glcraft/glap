#pragma once
#include "../common/utils.h"
#include <string_view>
namespace glap::v2::utils 
{
    template <glap::utils::Iterator<std::string_view> T>
    struct BiIterator {
        T begin;
        T end;
    };
    template <class T>
    BiIterator(T,T) -> BiIterator<T>;
}