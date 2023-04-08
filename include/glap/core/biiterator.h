#pragma once

#ifndef GLAP_MODULE
#include "base.h"
#include "utils.h"
#include <string_view>
#endif

GLAP_EXPORT namespace glap::impl
{
    template <Iterator<std::string_view> T>
    class BiIterator {
        T m_begin;
        T m_end;
    public:
        BiIterator(T begin, T end) : m_begin(begin), m_end(end)
        {}
        BiIterator(const BiIterator&) = default;
        BiIterator(BiIterator&&) = default;
        

        size_t size() const {
            return std::distance(m_begin, m_end);
        }
        bool empty() const {
            return m_begin == m_end;
        }
        T begin() const {
            return m_begin;
        }
        T end() const {
            return m_end;
        }
        T begin() {
            return m_begin;
        }
        T end() {
            return m_end;
        }
    };
    template <class T>
    BiIterator(T,T) -> BiIterator<T>;
}